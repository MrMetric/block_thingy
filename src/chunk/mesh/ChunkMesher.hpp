#pragma once

#include <vector>

#include <GL/glew.h>

class Chunk;

class ChunkMesher
{
	public:
		explicit ChunkMesher(const Chunk&);
		ChunkMesher(const ChunkMesher&) = delete;
		virtual ~ChunkMesher();

		virtual std::vector<GLbyte> make_mesh() = 0;

	protected:
		const Chunk& chunk;

		bool block_is_empty(int_fast16_t x, int_fast16_t y, int_fast16_t z) const;
};