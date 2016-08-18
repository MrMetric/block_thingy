#include "GreedyMesher.hpp"

#include <algorithm>
#include <stdint.h>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "block/Block.hpp"
#include "chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

using Graphics::Color;
using Position::BlockInChunk;

using surface_t = GreedyMesher::surface_t;
using u8vec3 = glm::tvec3<uint_fast8_t>;

struct Rectangle
{
	meshmap_key_t key;
	BlockInChunk::value_type x, z;
	BlockInChunk::value_type w, h;
};

enum class Plane
{
	XY,
	XZ,
	YZ,
};

enum class Side : int_fast8_t
{
	top = 1,
	bottom = -1,
};

static void add_surface(const Chunk&, meshmap_t&, surface_t&, Plane, Side);
static Rectangle yield_rectangle(surface_t&);
static void generate_surface(const Chunk&, surface_t&, u8vec3&, const u8vec3&, int_fast8_t);
static void add_face(mesh_t&, const mesh_vertex_coord_t&, const mesh_vertex_coord_t&, const mesh_vertex_coord_t&, const mesh_vertex_coord_t&);

meshmap_t GreedyMesher::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;

	add_surface(chunk, meshes, surface, Plane::XY, Side::top);
	add_surface(chunk, meshes, surface, Plane::XY, Side::bottom);
	add_surface(chunk, meshes, surface, Plane::XZ, Side::top);
	add_surface(chunk, meshes, surface, Plane::XZ, Side::bottom);
	add_surface(chunk, meshes, surface, Plane::YZ, Side::top);
	add_surface(chunk, meshes, surface, Plane::YZ, Side::bottom);

	return meshes;
}

void add_surface(const Chunk& chunk, meshmap_t& meshes, surface_t& surface, const Plane plane, const Side side)
{
	uint_fast8_t ix, iy, iz;
	if(plane == Plane::XY)
	{
		ix = 0;
		iy = 2;
		iz = 1;
	}
	else if(plane == Plane::XZ)
	{
		ix = 0;
		iy = 1;
		iz = 2;
	}
	else // must be YZ
	{
		ix = 1;
		iy = 0;
		iz = 2;
	}

	u8vec3 xyz;
	for(xyz[1] = 0; xyz[1] < CHUNK_SIZE; ++xyz[1])
	{
		generate_surface(chunk, surface, xyz, { ix, iy, iz }, static_cast<int_fast8_t>(side));

		while(true)
		{
			const Rectangle rekt = yield_rectangle(surface);
			if(std::get<0>(rekt.key) == BlockType::none)
			{
				break;
			}

			mesh_vertex_coord_t v1;
			mesh_vertex_coord_t v2;
			mesh_vertex_coord_t v3;
			mesh_vertex_coord_t v4;

			uint_fast8_t y1 = xyz[1];
			if(side == Side::top)
			{
				y1 += 1;
			}

			#define s(x) static_cast<mesh_vertex_coord_t::value_type>(x)
			v1[ix] = rekt.x;
			v1[iy] = y1;
			v1[iz] = rekt.z;

			v2[ix] = s(rekt.x + rekt.w);
			v2[iy] = y1;
			v2[iz] = rekt.z;

			v3[ix] = s(rekt.x + rekt.w);
			v3[iy] = y1;
			v3[iz] = s(rekt.z + rekt.h);

			v4[ix] = rekt.x;
			v4[iy] = y1;
			v4[iz] = s(rekt.z + rekt.h);
			#undef s

			mesh_t& mesh = meshes[rekt.key];
			if((plane == Plane::XZ && side == Side::top)
			|| (plane == Plane::XY && side == Side::bottom)
			|| (plane == Plane::YZ && side == Side::bottom))
			{
				add_face(mesh, v4, v3, v2, v1);
			}
			else
			{
				add_face(mesh, v1, v2, v3, v4);
			}
		}
	}
}

void generate_surface(const Chunk& chunk, surface_t& surface, u8vec3& xyz, const u8vec3& i, const int_fast8_t offset)
{
	for(xyz[0] = 0; xyz[0] < CHUNK_SIZE; ++xyz[0])
	{
		for(xyz[2] = 0; xyz[2] < CHUNK_SIZE; ++xyz[2])
		{
			const uint_fast8_t x = xyz[i.x];
			const uint_fast8_t y = xyz[i.y];
			const uint_fast8_t z = xyz[i.z];
			int_fast8_t o[] = {0, 0, 0};
			o[i.y] = offset;

			const Block::Block block = ChunkMesher::block_at(chunk, x, y, z);
			if(ChunkMesher::block_visible_from(chunk, block, x + o[0], y + o[1], z + o[2]))
			{
				surface[xyz[2]][xyz[0]] = { block.type(), ChunkMesher::light_at(chunk, x + o[0], y + o[1], z + o[2]) };
			}
			else
			{
				surface[xyz[2]][xyz[0]] = ChunkMesher::empty_key;
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
			const meshmap_key_t key = row[x];
			const BlockType type = std::get<0>(key);
			if(type != BlockType::none)
			{
				const BlockInChunk::value_type start_z = z;
				const BlockInChunk::value_type start_x = x;
				BlockInChunk::value_type w = 1;
				BlockInChunk::value_type h = 1;
				row[x] = ChunkMesher::empty_key;
				++x;
				while(x < CHUNK_SIZE && row[x] == key)
				{
					w += 1;
					row[x] = ChunkMesher::empty_key;
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
					std::fill_n(&row2[start_x], w2, ChunkMesher::empty_key);

					++z;
					h += 1;
				}
				return { key, start_x, start_z, w, h };
			}
		}
	}

	return { ChunkMesher::empty_key, 0, 0, 0, 0 };
}

void add_face(mesh_t& mesh, const mesh_vertex_coord_t& p1, const mesh_vertex_coord_t& p2, const mesh_vertex_coord_t& p3, const mesh_vertex_coord_t& p4)
{
	mesh.emplace_back(p1, p2, p3);
	mesh.emplace_back(p3, p4, p1);
}
