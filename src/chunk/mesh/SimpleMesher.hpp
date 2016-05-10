#pragma once
#include "ChunkMesher.hpp"

#include <stdint.h>

#include "position/BlockInChunk.hpp"

namespace Block
{
	class Block;
}

class SimpleMesher : public ChunkMesher
{
	public:
		explicit SimpleMesher(const Chunk&);

		meshmap_t make_mesh() override;

	private:
		void draw_cube(mesh_t&, const Block::Block& block, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
		void draw_face(mesh_t&, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type, uint_fast8_t face);
};
