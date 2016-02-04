#include "SimpleMesher.hpp"

#include <vector>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../Coords.hpp"
#include "../../Cube.hpp"
#include "../../World.hpp"

SimpleMesher::SimpleMesher(const Chunk& chunk)
	:
	ChunkMesher(chunk)
{
}

std::vector<GLfloat> SimpleMesher::make_mesh()
{
	std::vector<GLfloat> vertexes;
	for(BlockInChunk_type x = 0; x < CHUNK_SIZE; ++x)
	{
		for(BlockInChunk_type y = 0; y < CHUNK_SIZE; ++y)
		{
			for(BlockInChunk_type z = 0; z < CHUNK_SIZE; ++z)
			{
				if(chunk.get_block(x, y, z).type() == 0)
				{
					continue;
				}
				draw_cube(vertexes, x, y, z);
			}
		}
	}
	return vertexes;
}

void SimpleMesher::draw_cube(std::vector<GLfloat>& vertexes, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z)
{
	// front
	if(block_is_empty(x, y, z - 1))
	{
		draw_face(vertexes, x, y, z, 0);
	}
	// back
	if(block_is_empty(x, y, z + 1))
	{
		draw_face(vertexes, x, y, z, 1);
	}
	// top
	if(block_is_empty(x, y + 1, z))
	{
		draw_face(vertexes, x, y, z, 2);
	}
	// bottom
	if(block_is_empty(x, y - 1, z))
	{
		draw_face(vertexes, x, y, z, 3);
	}
	// right (?)
	if(block_is_empty(x - 1, y, z))
	{
		draw_face(vertexes, x, y, z, 4);
	}
	// left (?)
	if(block_is_empty(x + 1, y, z))
	{
		draw_face(vertexes, x, y, z, 5);
	}
}

void SimpleMesher::draw_face(std::vector<GLfloat>& vertexes, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t face)
{
	auto offset = face * 6;
	for(uint_fast8_t i = 0; i < 6; ++i)
	{
		uint_fast8_t element = 3 * Cube::cube_elements[offset++];
		vertexes.push_back(Cube::cube_vertex[element++] + x);
		vertexes.push_back(Cube::cube_vertex[element++] + y);
		vertexes.push_back(Cube::cube_vertex[element++] + z);
	}
}

bool SimpleMesher::block_is_empty(int_fast16_t x, int_fast16_t y, int_fast16_t z) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		auto chunk_pos = chunk.get_position();
		int64_t bx = chunk_pos.x * CHUNK_SIZE + x;
		int64_t by = chunk_pos.y * CHUNK_SIZE + y;
		int64_t bz = chunk_pos.z * CHUNK_SIZE + z;
		return chunk.get_owner()->get_block(Position::BlockInWorld(bx, by, bz)).type() == 0;
	}
	return chunk.get_block(x, y, z).type() == 0;
}
