#include "SimpleMesher.hpp"

#include <stdint.h>

#include "Cube.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

static void draw_cube(const Chunk&, meshmap_t&, const Block::Block& block, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
static void draw_face(mesh_t&, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type, uint_fast8_t face);

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
				draw_cube(chunk, meshes, block, x, y, z);
			}
		}
	}
	return meshes;
}

void draw_cube(const Chunk& chunk, meshmap_t& meshes, const Block::Block& block, const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
{
	// front
	if(ChunkMesher::block_visible_from(chunk, block, x, y, z - 1))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x, y, z - 1) };
		draw_face(meshes[key], x, y, z, 0);
	}
	// back
	if(ChunkMesher::block_visible_from(chunk, block, x, y, z + 1))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x, y, z + 1) };
		draw_face(meshes[key], x, y, z, 1);
	}
	// top
	if(ChunkMesher::block_visible_from(chunk, block, x, y + 1, z))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x, y + 1, z) };
		draw_face(meshes[key], x, y, z, 2);
	}
	// bottom
	if(ChunkMesher::block_visible_from(chunk, block, x, y - 1, z))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x, y - 1, z) };
		draw_face(meshes[key], x, y, z, 3);
	}
	// right (?)
	if(ChunkMesher::block_visible_from(chunk, block, x - 1, y, z))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x - 1, y, z) };
		draw_face(meshes[key], x, y, z, 4);
	}
	// left (?)
	if(ChunkMesher::block_visible_from(chunk, block, x + 1, y, z))
	{
		const meshmap_key_t key = { block.type(), ChunkMesher::light_at(chunk, x + 1, y, z) };
		draw_face(meshes[key], x, y, z, 5);
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
