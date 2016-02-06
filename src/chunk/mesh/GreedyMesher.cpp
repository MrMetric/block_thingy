#include "GreedyMesher.hpp"

#include <iostream>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../World.hpp"

GreedyMesher::GreedyMesher(const Chunk& chunk)
	:
	ChunkMesher(chunk)
{
}

void add_face(std::array<GLbyte, 3> p1, std::array<GLbyte, 3> p2, std::array<GLbyte, 3> p3, std::array<GLbyte, 3> p4, std::vector<GLbyte>& vertexes)
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

// derived from https://github.com/mikolalysenko/mikolalysenko.github.com/blob/gh-pages/MinecraftMeshes/js/greedy.js
// for details, see http://0fps.net/2012/06/30/meshing-in-a-minecraft-game/
std::vector<GLbyte> GreedyMesher::make_mesh()
{
	std::vector<GLbyte> vertexes;

	std::unique_ptr<block_id_type[]> map = std::make_unique<block_id_type[]>(CHUNK_SIZE * CHUNK_SIZE);

	for(uint_fast8_t dim = 0; dim < 3; ++dim)
	{
		int u = (dim + 1) % 3;
		int v = (dim + 2) % 3;
		int_fast8_t pos[] = {0,0,0};
		uint_fast8_t q[] = {0,0,0};
		q[dim] = 1;
		for(pos[dim] = -1; pos[dim] < CHUNK_SIZE; )
		{
			uint_fast16_t map_i = 0;
			for(pos[v] = 0; pos[v] < CHUNK_SIZE; ++pos[v])
			{
				for(pos[u] = 0; pos[u] < CHUNK_SIZE; ++pos[u])
				{
					#define s(a) static_cast<BlockInChunk_type>(a)
					block_id_type type1 = 0;
					if(pos[dim] >= 0)
					{
						BlockInChunk_type xx = s(pos[0]);
						BlockInChunk_type yy = s(pos[1]);
						BlockInChunk_type zz = s(pos[2]);
						type1 = chunk.get_block(xx, yy, zz).type();
					}
					block_id_type type2 = 0;
					if(pos[dim] < CHUNK_SIZE - 1)
					{
						BlockInChunk_type xx = s(pos[0] + q[0]);
						BlockInChunk_type yy = s(pos[1] + q[1]);
						BlockInChunk_type zz = s(pos[2] + q[2]);
						type2 = chunk.get_block(xx, yy, zz).type();
					}
					#undef s
					// TODO: fix this for multiple block types
					map[map_i++] = type1 != type2;
				}
			}

			++pos[dim];
			map_i = 0;
			for(int_fast8_t j = 0; j < CHUNK_SIZE; ++j)
			{
				for(int_fast8_t i = 0; i < CHUNK_SIZE; )
				{
					uint_fast8_t width = 1;
					uint_fast8_t height = 1;
					if(map[map_i] != 0)
					{
						for(; (map[map_i + width] != 0) && (i + width < CHUNK_SIZE); ++width)
						{
						}
						for(; j + height < CHUNK_SIZE; ++height)
						{
							for(uint_fast8_t k = 0; k < width; ++k)
							{
								if(map[map_i + k + height*CHUNK_SIZE] == 0)
								{
									goto height_loop_end;
								}
							}
						}
						height_loop_end:;

						pos[u] = i;
						pos[v] = j;
						uint_fast8_t du[] = {0,0,0}; du[u] = width;
						uint_fast8_t dv[] = {0,0,0}; dv[v] = height;

						#define s(a) static_cast<GLbyte>(a)
						std::array<GLbyte, 3> vert1 { s(pos[0]            ), s(pos[1]            ), s(pos[2]            ) };
						std::array<GLbyte, 3> vert2 { s(pos[0]+du[0]      ), s(pos[1]+du[1]      ), s(pos[2]+du[2]      ) };
						std::array<GLbyte, 3> vert3 { s(pos[0]+du[0]+dv[0]), s(pos[1]+du[1]+dv[1]), s(pos[2]+du[2]+dv[2]) };
						std::array<GLbyte, 3> vert4 { s(pos[0]      +dv[0]), s(pos[1]      +dv[1]), s(pos[2]      +dv[2]) };
						#undef s

						// keep the faces the right way around for back-face culling
						if(block_is_empty(pos[0], pos[1], pos[2], false))
						{
							add_face(
								vert1,
								vert2,
								vert3,
								vert4,
								vertexes
							);
						}
						else
						{
							add_face(
								vert4,
								vert3,
								vert2,
								vert1,
								vertexes
							);
						}

						for(uint_fast8_t map_y = 0; map_y < height; ++map_y)
						{
							for(uint_fast8_t map_x = 0; map_x < width; ++map_x)
							{
								map[map_i + map_y*CHUNK_SIZE + map_x] = 0;
							}
						}
					}
					i += width;
					map_i += width;
				}
			}
		}
	}

	return vertexes;
}
