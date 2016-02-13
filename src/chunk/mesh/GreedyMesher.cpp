#include "GreedyMesher.hpp"

#include <array>
#include <cstdint>

#include <glad/glad.h>

#include "../Chunk.hpp"
#include "../../Block.hpp"

namespace GreedyMesherPrivate
{
	struct Rectangle
	{
		BlockType type;
		uint8_t x, z, w, h;
	};

	enum class Plane
	{
		XY,
		XZ,
		YZ,
	};

	enum class Side
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

static void add_face(std::array<GLubyte, 3> p1, std::array<GLubyte, 3> p2, std::array<GLubyte, 3> p3, std::array<GLubyte, 3> p4, std::vector<GLubyte>& vertexes)
{
	vertexes.push_back(p1[0]);
	vertexes.push_back(p1[1]);
	vertexes.push_back(p1[2]);
	vertexes.push_back(p2[0]);
	vertexes.push_back(p2[1]);
	vertexes.push_back(p2[2]);
	vertexes.push_back(p3[0]);
	vertexes.push_back(p3[1]);
	vertexes.push_back(p3[2]);

	vertexes.push_back(p3[0]);
	vertexes.push_back(p3[1]);
	vertexes.push_back(p3[2]);
	vertexes.push_back(p4[0]);
	vertexes.push_back(p4[1]);
	vertexes.push_back(p4[2]);
	vertexes.push_back(p1[0]);
	vertexes.push_back(p1[1]);
	vertexes.push_back(p1[2]);
}

Rectangle GreedyMesher::yield_rectangle()
{
	uint8_t start_z;
	uint8_t start_x;
	uint8_t w = 0;
	uint8_t h = 0;
	for(uint_fast32_t z = 0; z < surface.size(); ++z)
	{
		std::vector<BlockType>& row = surface[z];
		for(uint_fast32_t x = 0; x < row.size(); ++x)
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
				while(x < row.size() && row[x] == type)
				{
					w += 1;
					row[x] = BlockType::none;
					++x;
				}
				++z;
				while(z < surface.size())
				{
					x = start_x;
					std::vector<BlockType>& row2 = surface[z];

					if(row2[x] != type)
					{
						break;
					}
					uint_fast32_t w2 = 0;
					do
					{
						w2 += 1;
						++x;
					}
					while(x < row2.size() && w2 < w && row2[x] == type);

					if(w2 == w)
					{
						for(uint_fast32_t x2 = start_x; x2 < start_x + w2; ++x2)
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

void GreedyMesher::add_surface(std::vector<GLubyte>& vertexes, Plane plane, Side side)
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
		for(xyz[0] = 0; xyz[0] < CHUNK_SIZE; ++xyz[0])
		{
			for(xyz[2] = 0; xyz[2] < CHUNK_SIZE; ++xyz[2])
			{
				uint_fast8_t x = xyz[ix];
				uint_fast8_t y = xyz[iy];
				uint_fast8_t z = xyz[iz];
				int offset = static_cast<int>(side);
				int_fast8_t o[] = {0, 0, 0};
				o[iy] = offset;

				bool empty = block_is_invisible_not_none(x + o[0], y + o[1], z + o[2]);
				if(empty && !block_is_invisible(x, y, z))
				{
					surface[xyz[2]][xyz[0]] = chunk.get_block(x, y, z).type();
				}
				else
				{
					surface[xyz[2]][xyz[0]] = BlockType::none;
				}
			}
		}

		while(true)
		{
			Rectangle rekt = yield_rectangle();
			if(rekt.x == 0 && rekt.z == 0 && rekt.w == 0 && rekt.h == 0)
			{
				break;
			}

			std::array<GLubyte, 3> v1;
			std::array<GLubyte, 3> v2;
			std::array<GLubyte, 3> v3;
			std::array<GLubyte, 3> v4;

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

std::vector<GLubyte> GreedyMesher::make_mesh()
{
	std::vector<GLubyte> vertexes;

	add_surface(vertexes, Plane::XY, Side::top);
	add_surface(vertexes, Plane::XY, Side::bottom);
	add_surface(vertexes, Plane::XZ, Side::top);
	add_surface(vertexes, Plane::XZ, Side::bottom);
	add_surface(vertexes, Plane::YZ, Side::top);
	add_surface(vertexes, Plane::YZ, Side::bottom);

	return vertexes;
}
