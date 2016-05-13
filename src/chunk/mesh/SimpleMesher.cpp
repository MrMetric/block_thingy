#include "SimpleMesher.hpp"

#include <stdint.h>

#include "Cube.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

static void draw_cube(const Chunk&, mesh_t&, const Block::Block& block, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
static void draw_face(mesh_t&, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type, uint_fast8_t face);

SimpleMesher::SimpleMesher()
{
}

meshmap_t SimpleMesher::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;
	for(BlockInChunk_type x = 0; x < CHUNK_SIZE; ++x)
	{
		for(BlockInChunk_type y = 0; y < CHUNK_SIZE; ++y)
		{
			for(BlockInChunk_type z = 0; z < CHUNK_SIZE; ++z)
			{
				const Block::Block& block = block_at(chunk, x, y, z);
				if(block.is_invisible())
				{
					continue;
				}
				BlockType type = block.type();
				mesh_t& mesh = meshes[type];
				draw_cube(chunk, mesh, block, x, y, z);
			}
		}
	}
	return meshes;
}

void draw_cube(const Chunk& chunk, mesh_t& mesh, const Block::Block& block, const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	auto block_visible_from = [&chunk](const Block::Block& block, int_fast16_t x, int_fast16_t y, int_fast16_t z)
	{
		const Block::Block& sibling = ChunkMesher::block_at(chunk, x, y, z);
		return !sibling.is_opaque() && block.type() != sibling.type();
	};

	// front
	if(block_visible_from(block, x, y, z - 1))
	{
		draw_face(mesh, x, y, z, 0);
	}
	// back
	if(block_visible_from(block, x, y, z + 1))
	{
		draw_face(mesh, x, y, z, 1);
	}
	// top
	if(block_visible_from(block, x, y + 1, z))
	{
		draw_face(mesh, x, y, z, 2);
	}
	// bottom
	if(block_visible_from(block, x, y - 1, z))
	{
		draw_face(mesh, x, y, z, 3);
	}
	// right (?)
	if(block_visible_from(block, x - 1, y, z))
	{
		draw_face(mesh, x, y, z, 4);
	}
	// left (?)
	if(block_visible_from(block, x + 1, y, z))
	{
		draw_face(mesh, x, y, z, 5);
	}
}

void draw_face(mesh_t& mesh, const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, const uint_fast8_t face)
{
	auto offset = face * 6;
	for(uint_fast8_t i = 0; i < 2; ++i)
	{
		mesh_triangle_t tri;
		for(uint_fast8_t j = 0; j < 3; ++j)
		{
			uint_fast8_t element = 3 * Cube::cube_elements[offset++];
			tri[j].x = Cube::cube_vertex[element++] + x;
			tri[j].y = Cube::cube_vertex[element++] + y;
			tri[j].z = Cube::cube_vertex[element++] + z;
		}
		mesh.push_back(tri);
	}
}
