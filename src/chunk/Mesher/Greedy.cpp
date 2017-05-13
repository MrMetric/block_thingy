#include "Greedy.hpp"

#include <algorithm>
#include <cassert>
#include <stdint.h>

#include <glm/vec3.hpp>

#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "block/Enum/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

using Block::Enum::Face;
using Position::BlockInChunk;

namespace Mesher {

using surface_t = Greedy::surface_t;

struct Rectangle
{
	BlockType type;
	BlockInChunk::value_type x, z;
	BlockInChunk::value_type w, h;
	Graphics::Color light;
};

static void add_surface(const Chunk&, meshmap_t&, surface_t&, Face, Side);
static Rectangle yield_rectangle(surface_t&);
static void generate_surface(const Chunk&, surface_t&, u8vec3&, const u8vec3&, Side);

meshmap_t Greedy::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;

	add_surface(chunk, meshes, surface, Face::back  , Side::top);		// Plane::XY
	add_surface(chunk, meshes, surface, Face::front , Side::bottom);	// Plane::XY
	add_surface(chunk, meshes, surface, Face::top   , Side::top);		// Plane::XZ
	add_surface(chunk, meshes, surface, Face::bottom, Side::bottom);	// Plane::XZ
	add_surface(chunk, meshes, surface, Face::left  , Side::top);		// Plane::YZ
	add_surface(chunk, meshes, surface, Face::right , Side::bottom);	// Plane::YZ

	return meshes;
}

void add_surface
(
	const Chunk& chunk,
	meshmap_t& meshes,
	surface_t& surface,
	const Face face,
	const Side side
)
{
	const u8vec3 i = Base::get_i(face);

	u8vec3 pos;
	for(pos[1] = 0; pos[1] < CHUNK_SIZE; ++pos[1])
	{
		generate_surface(chunk, surface, pos, i, side);

		while(true)
		{
			const Rectangle rekt = yield_rectangle(surface);
			if(rekt.type == BlockType::none)
			{
				break;
			}

			u8vec3 xyz;
			xyz[i.x] = rekt.x;
			xyz[i.y] = pos.y;
			xyz[i.z] = rekt.z;

			Base::add_face(meshes[rekt.type], xyz, face, rekt.w, rekt.h, rekt.light);
		}
	}
}

void generate_surface
(
	const Chunk& chunk,
	surface_t& surface,
	u8vec3& pos,
	const u8vec3& i,
	const Side side
)
{
	const auto offset = static_cast<int8_t>(side);
	for(pos[0] = 0; pos[0] < CHUNK_SIZE; ++pos[0])
	{
		for(pos[2] = 0; pos[2] < CHUNK_SIZE; ++pos[2])
		{
			const auto x = pos[i.x];
			const auto y = pos[i.y];
			const auto z = pos[i.z];
			int8_t o[] = {0, 0, 0};
			o[i.y] = offset;

			const Block::Base& block = Base::block_at(chunk, x, y, z);
			const Graphics::Color light = Base::light_at(chunk, x + o[0], y + o[1], z + o[2]);
			if(Base::block_visible_from(chunk, block, x + o[0], y + o[1], z + o[2]))
			{
				surface[pos[2]][pos[0]] = { block.type(), light };
			}
			else
			{
				surface[pos[2]][pos[0]] = { BlockType::none, light };
			}
		}
	}
}

Rectangle yield_rectangle(surface_t& surface)
{
	for(BlockInChunk::value_type z = 0; z < CHUNK_SIZE; ++z)
	{
		surface_t::value_type& row = surface[z];
		for(BlockInChunk::value_type x = 0; x < CHUNK_SIZE; ++x)
		{
			const auto key = row[x];
			const BlockType type = std::get<0>(key);
			if(type == BlockType::none)
			{
				continue;
			}
			const BlockInChunk::value_type start_z = z;
			const BlockInChunk::value_type start_x = x;
			BlockInChunk::value_type w = 1;
			BlockInChunk::value_type h = 1;
			std::get<0>(row[x]) = BlockType::none;
			++x;
			while(x < CHUNK_SIZE && row[x] == key)
			{
				w += 1;
				std::get<0>(row[x]) = BlockType::none;
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
				BlockInChunk::value_type w2 = 0;
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
				for(size_t i = start_x; i < start_x + w2; ++i)
				{
					std::get<0>(row2[start_x]) = BlockType::none;
				}

				++z;
				h += 1;
			}
			return
			{
				std::get<0>(key), // block type
				start_x, start_z,
				w, h,
				std::get<1>(key), // light
			};
		}
	}

	return { BlockType::none, 0, 0, 0, 0, {0} };
}

}
