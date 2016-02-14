#pragma once

#include <map>
#include <vector>

#include <glad/glad.h>

#include "../../BlockType.hpp"

class Block;
class Chunk;

using mesh_t = std::map<BlockType, std::vector<GLubyte>>;

class ChunkMesher
{
	public:
		explicit ChunkMesher(const Chunk&);
		ChunkMesher(const ChunkMesher&) = delete;
		virtual ~ChunkMesher();

		virtual mesh_t make_mesh() = 0;

	protected:
		const Chunk& chunk;

		const Block block_at(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_invisible(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_invisible_not_none(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
};
