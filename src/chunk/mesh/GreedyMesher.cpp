#include "GreedyMesher.hpp"

#include <cstdint>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../position/BlockInChunk.hpp"

namespace GreedyMesherPrivate
{
	struct Rectangle
	{
		BlockType type;
		BlockInChunk_type x, z, w, h;
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
}
using namespace GreedyMesherPrivate;

GreedyMesher::GreedyMesher(const Chunk& chunk)
	:
	ChunkMesher(chunk)
{
	for(uint_fast32_t i = 0; i < CHUNK_SIZE; ++i)
	{
		surface.emplace_back(CHUNK_SIZE);
	}
}

meshmap_t GreedyMesher::make_mesh()
{
	meshmap_t meshes;

	add_surface(meshes, Plane::XY, Side::top);
	add_surface(meshes, Plane::XY, Side::bottom);
	add_surface(meshes, Plane::XZ, Side::top);
	add_surface(meshes, Plane::XZ, Side::bottom);
	add_surface(meshes, Plane::YZ, Side::top);
	add_surface(meshes, Plane::YZ, Side::bottom);

	return meshes;
}

void GreedyMesher::add_surface(meshmap_t& meshes, Plane plane, Side side)
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

	glm::tvec3<uint_fast8_t> xyz;
	for(xyz[1] = 0; xyz[1] < CHUNK_SIZE; ++xyz[1])
	{
		generate_surface(xyz, ix, iy, iz, static_cast<int_fast8_t>(side));

		while(true)
		{
			Rectangle rekt = yield_rectangle();
			if(rekt.type == BlockType::none)
			{
				break;
			}

			mesh_vertex_coord_t v1;
			mesh_vertex_coord_t v2;
			mesh_vertex_coord_t v3;
			mesh_vertex_coord_t v4;

			uint8_t y1 = xyz[1];
			if(side == Side::top)
			{
				y1 += 1;
			}

			#define s(x) static_cast<GLubyte>(x)
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

			mesh_t& vertexes = meshes[rekt.type];
			if((plane == Plane::XZ && side == Side::top)
			|| (plane == Plane::XY && side == Side::bottom)
			|| (plane == Plane::YZ && side == Side::bottom))
			{
				add_face(v4, v3, v2, v1, vertexes);
			}
			else
			{
				add_face(v1, v2, v3, v4, vertexes);
			}
		}
	}
}

void GreedyMesher::generate_surface(glm::tvec3<uint_fast8_t>& xyz, const uint_fast8_t ix, const uint_fast8_t iy, const uint_fast8_t iz, const int_fast8_t offset)
{
	for(xyz[0] = 0; xyz[0] < CHUNK_SIZE; ++xyz[0])
	{
		for(xyz[2] = 0; xyz[2] < CHUNK_SIZE; ++xyz[2])
		{
			uint_fast8_t x = xyz[ix];
			uint_fast8_t y = xyz[iy];
			uint_fast8_t z = xyz[iz];
			int_fast8_t o[] = {0, 0, 0};
			o[iy] = offset;

			bool empty = block_is_invisible_not_none(x + o[0], y + o[1], z + o[2]);
			if(empty && !block_is_invisible(x, y, z))
			{
				surface[xyz[2]][xyz[0]] = chunk.get_block_const({x, y, z}).type();
			}
			else
			{
				surface[xyz[2]][xyz[0]] = BlockType::none;
			}
		}
	}
}

Rectangle GreedyMesher::yield_rectangle()
{
	BlockInChunk_type start_z;
	BlockInChunk_type start_x;
	BlockInChunk_type w = 0;
	BlockInChunk_type h = 0;
	const size_t surface_size = surface.size();
	for(BlockInChunk_type z = 0; z < surface_size; ++z)
	{
		std::vector<BlockType>& row = surface[z];
		const size_t row_size = row.size();
		for(BlockInChunk_type x = 0; x < row_size; ++x)
		{
			const BlockType type = row[x];
			if(type != BlockType::none)
			{
				start_z = z;
				start_x = x;
				w = 1;
				h = 1;
				row[x] = BlockType::none;
				++x;
				while(x < row_size && row[x] == type)
				{
					w += 1;
					row[x] = BlockType::none;
					++x;
				}
				++z;
				while(z < surface_size)
				{
					x = start_x;
					std::vector<BlockType>& row2 = surface[z];

					if(row2[x] != type)
					{
						break;
					}
					BlockInChunk_type w2 = 0;
					const size_t row2_size = row2.size();
					do
					{
						w2 += 1;
						++x;
					}
					while(x < row2_size && w2 < w && row2[x] == type);

					if(w2 == w)
					{
						for(BlockInChunk_type x2 = start_x; x2 < start_x + w2; ++x2)
						{
							row2[x2] = BlockType::none;
						}
					}
					else
					{
						break;
					}

					++z;
					h += 1;
				}
				return { type, start_x, start_z, w, h };
			}
		}
	}

	return { BlockType::none, 0, 0, 0, 0 };
}

void GreedyMesher::add_face(const mesh_vertex_coord_t& p1, const mesh_vertex_coord_t& p2, const mesh_vertex_coord_t& p3, const mesh_vertex_coord_t& p4, mesh_t& vertexes)
{
	vertexes.emplace_back(p1, p2, p3);
	vertexes.emplace_back(p3, p4, p1);
}
