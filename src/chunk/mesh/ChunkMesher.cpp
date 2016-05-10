#include "ChunkMesher.hpp"

#include <stdint.h>

#include "World.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

ChunkMesher::ChunkMesher(const Chunk& chunk)
	:
	chunk(chunk)
{
}

ChunkMesher::~ChunkMesher()
{
}

const Block::Block& ChunkMesher::block_at(const int_fast16_t x, const int_fast16_t y, const int_fast16_t z, const bool allow_out_of_bounds) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		if(!allow_out_of_bounds)
		{
			static const Block::Block none = Block::Block(BlockType::none);
			return none;
		}
		Position::ChunkInWorld chunk_pos = chunk.get_position();
		Position::BlockInWorld block_pos(chunk_pos, {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_block_const(block_pos);
	}
	#define s(a) static_cast<BlockInChunk_type>(a)
	return chunk.get_block_const(s(x), s(y), s(z));
	#undef s
}

bool ChunkMesher::block_is_invisible(const int_fast16_t x, const int_fast16_t y, const int_fast16_t z, const bool allow_out_of_bounds) const
{
	return block_at(x, y, z, allow_out_of_bounds).is_invisible();
}

bool ChunkMesher::block_is_opaque(const int_fast16_t x, const int_fast16_t y, const int_fast16_t z, const bool allow_out_of_bounds) const
{
	return block_at(x, y, z, allow_out_of_bounds).is_opaque();
}
