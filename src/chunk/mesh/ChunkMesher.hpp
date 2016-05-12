#pragma once

#include <map>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>

#include "block/BlockType.hpp"
#include "graphics/primitive.hpp"

class Chunk;
namespace Block
{
	class Block;
}

using mesh_triangle_t = triangle_t<GLubyte>;
using mesh_vertex_coord_t = mesh_triangle_t::value_type;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_t = std::map<BlockType, mesh_t>;

class ChunkMesher
{
	public:
		ChunkMesher();
		virtual ~ChunkMesher();

		ChunkMesher(ChunkMesher&&) = delete;
		ChunkMesher(const ChunkMesher&) = delete;
		void operator=(const ChunkMesher&) = delete;

		virtual meshmap_t make_mesh(const Chunk&) = 0;

	protected:
		const Block::Block& block_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_invisible(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_opaque(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
};
