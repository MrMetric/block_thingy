#include "Greedy.hpp"

#include <array>
#include <stdint.h>
#include <tuple>

#include "block/base.hpp"
#include "block/enums/Face.hpp"
#include "position/block_in_chunk.hpp"

namespace block_thingy::mesher {

using block::enums::Face;
using position::block_in_chunk;

using surface_t = std::array<std::array<std::tuple<meshmap_key_t, uint16_t, uint8_t>, CHUNK_SIZE>, CHUNK_SIZE>;

struct Rectangle
{
	meshmap_key_t key;
	block_in_chunk::value_type x, z;
	block_in_chunk::value_type w, h;
	uint16_t tex_index;
	uint8_t rotation;
};

static void add_surface(const Chunk&, meshmap_t&, surface_t&, Face);
static Rectangle yield_rectangle(surface_t&);
static void generate_surface(const Chunk&, surface_t&, u8vec3&, const u8vec3&, Face);

meshmap_t greedy::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;

	surface_t surface;
	add_surface(chunk, meshes, surface, Face::right );
	add_surface(chunk, meshes, surface, Face::left  );
	add_surface(chunk, meshes, surface, Face::top   );
	add_surface(chunk, meshes, surface, Face::bottom);
	add_surface(chunk, meshes, surface, Face::front );
	add_surface(chunk, meshes, surface, Face::back  );

	return meshes;
}

void add_surface
(
	const Chunk& chunk,
	meshmap_t& meshes,
	surface_t& surface,
	const Face face
)
{
	const u8vec3 i = base::get_i(face);

	u8vec3 pos;
	for(pos[1] = 0; pos[1] < CHUNK_SIZE; ++pos[1])
	{
		generate_surface(chunk, surface, pos, i, face);

		while(true)
		{
			const Rectangle rekt = yield_rectangle(surface);
			if(rekt.key.shader_path.empty())
			{
				break;
			}

			u8vec3 xyz;
			xyz[i.x] = rekt.x;
			xyz[i.y] = pos.y;
			xyz[i.z] = rekt.z;

			base::add_face(meshes[rekt.key], xyz, face, rekt.w, rekt.h, rekt.tex_index, rekt.rotation);
		}
	}
}

void generate_surface
(
	const Chunk& chunk,
	surface_t& surface,
	u8vec3& pos,
	const u8vec3& i,
	const Face face
)
{
	const Side side = base::to_side(face);
	const auto offset = static_cast<int8_t>(side);
	for(pos[0] = 0; pos[0] < CHUNK_SIZE; ++pos[0])
	{
		for(pos[2] = 0; pos[2] < CHUNK_SIZE; ++pos[2])
		{
			const auto x = pos[i.x];
			const auto y = pos[i.y];
			const auto z = pos[i.z];
			int8_t o[] {0, 0, 0};
			o[i.y] = offset;

			const block::base& block = base::block_at(chunk, x, y, z);
			if(base::block_visible_from(chunk, block, x + o[0], y + o[1], z + o[2]))
			{
				const auto tex = block.texture_info(face);
				surface[pos[2]][pos[0]] =
				{
					{
						block.shader_path(face),
						block.is_translucent(),
						tex.unit,
					},
					tex.index,
					block.rotation(face),
				};
			}
			else
			{
				surface[pos[2]][pos[0]] =
				{
					{
						{},
						false,
						0,
					},
					0,
					0,
				};
			}
		}
	}
}

Rectangle yield_rectangle(surface_t& surface)
{
	for(block_in_chunk::value_type z = 0; z < CHUNK_SIZE; ++z)
	{
		surface_t::value_type& row = surface[z];
		for(block_in_chunk::value_type x = 0; x < CHUNK_SIZE; ++x)
		{
			const auto key = row[x];
			const fs::path shader_path = std::get<0>(key).shader_path;
			if(shader_path.empty())
			{
				continue;
			}
			const block_in_chunk::value_type start_z = z;
			const block_in_chunk::value_type start_x = x;
			block_in_chunk::value_type w = 1;
			block_in_chunk::value_type h = 1;
			std::get<0>(row[x]).shader_path.clear();
			++x;
			while(x < CHUNK_SIZE && row[x] == key)
			{
				w += 1;
				std::get<0>(row[x]).shader_path.clear();
				++x;
			}
			++z;
			while(z < CHUNK_SIZE)
			{
				x = start_x;
				surface_t::value_type& row2 = surface[z];

				if(row2[x] != key)
				{
					break;
				}
				block_in_chunk::value_type w2 = 0;
				do
				{
					w2 += 1;
					++x;
				}
				while(x < CHUNK_SIZE && w2 < w && row2[x] == key);

				if(w2 != w)
				{
					break;
				}
				for(block_in_chunk::value_type i = start_x; i < start_x + w2; ++i)
				{
					std::get<0>(row2[start_x]).shader_path.clear();
				}

				++z;
				h += 1;
			}
			return
			{
				{
					std::get<0>(key).shader_path,
					std::get<0>(key).is_translucent,
					std::get<0>(key).tex_unit,
				},
				start_x, start_z,
				w, h,
				std::get<1>(key), // tex_index
				std::get<2>(key), // rotation
			};
		}
	}

	return
	{
		{
			{},
			false,
			0,
		},
		0, 0,
		0, 0,
		0,
		0,
	};
}

}
