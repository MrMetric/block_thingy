#include "Simple.hpp"

#include <stdint.h>

#include "Cube.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "block/Enum/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

using Block::Enum::Face;
using Position::BlockInChunk;

namespace Mesher {

static void draw_cube(const Chunk&, meshmap_t&, const Block::Base& block, BlockInChunk::value_type, BlockInChunk::value_type, BlockInChunk::value_type);
static void draw_face(mesh_t&, BlockInChunk::value_type, BlockInChunk::value_type, BlockInChunk::value_type, Face);

meshmap_t Simple::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;
	for(BlockInChunk::value_type x = 0; x < CHUNK_SIZE; ++x)
	{
		for(BlockInChunk::value_type y = 0; y < CHUNK_SIZE; ++y)
		{
			for(BlockInChunk::value_type z = 0; z < CHUNK_SIZE; ++z)
			{
				const Block::Base& block = block_at(chunk, x, y, z);
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

void draw_cube(const Chunk& chunk, meshmap_t& meshes, const Block::Base& block, const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z)
{
	if(Base::block_visible_from(chunk, block, x, y, z - 1))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x, y, z - 1) };
		draw_face(meshes[key], x, y, z, Face::front);
	}
	if(Base::block_visible_from(chunk, block, x, y, z + 1))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x, y, z + 1) };
		draw_face(meshes[key], x, y, z, Face::back);
	}
	if(Base::block_visible_from(chunk, block, x, y + 1, z))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x, y + 1, z) };
		draw_face(meshes[key], x, y, z, Face::top);
	}
	if(Base::block_visible_from(chunk, block, x, y - 1, z))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x, y - 1, z) };
		draw_face(meshes[key], x, y, z, Face::bottom);
	}
	if(Base::block_visible_from(chunk, block, x - 1, y, z))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x - 1, y, z) };
		draw_face(meshes[key], x, y, z, Face::right);
	}
	if(Base::block_visible_from(chunk, block, x + 1, y, z))
	{
		const meshmap_key_t key = { block.type(), Base::light_at(chunk, x + 1, y, z) };
		draw_face(meshes[key], x, y, z, Face::left);
	}
}

void draw_face(mesh_t& mesh, const BlockInChunk::value_type x, const BlockInChunk::value_type y, const BlockInChunk::value_type z, const Face face)
{
	auto offset = static_cast<std::size_t>(face) * 6;
	for(uint_fast8_t i = 0; i < 2; ++i)
	{
		mesh_triangle_t tri;
		for(uint_fast8_t j = 0; j < 3; ++j)
		{
			uint_fast8_t element = 3 * Cube::cube_elements[offset++];
			tri[j].x = Cube::cube_vertex[element++] + x;
			tri[j].y = Cube::cube_vertex[element++] + y;
			tri[j].z = Cube::cube_vertex[element++] + z;
			tri[j].w = static_cast<mesh_vertex_coord_t::value_type>(face);
		}
		mesh.push_back(tri);
	}
}

}
