#include "ChunkMesher.hpp"

#include <cstdint>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../BlockType.hpp"
#include "../../World.hpp"
#include "../../position/BlockInChunk.hpp"
#include "../../position/BlockInWorld.hpp"
#include "../../position/ChunkInWorld.hpp"

ChunkMesher::ChunkMesher(const Chunk& chunk)
	:
	chunk(chunk)
{
}

ChunkMesher::~ChunkMesher()
{
}

const Block& ChunkMesher::block_at(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		if(!allow_out_of_bounds)
		{
			static const Block none = Block(BlockType::none);
			return none;
		}
		Position::ChunkInWorld chunk_pos = chunk.get_position();
		BlockInWorld_type bx = chunk_pos.x * CHUNK_SIZE + x;
		BlockInWorld_type by = chunk_pos.y * CHUNK_SIZE + y;
		BlockInWorld_type bz = chunk_pos.z * CHUNK_SIZE + z;
		return chunk.get_owner()->get_block_const({bx, by, bz});
	}
	#define s(a) static_cast<BlockInChunk_type>(a)
	return chunk.get_block_const(s(x), s(y), s(z));
	#undef s
}

bool ChunkMesher::block_is_invisible(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds) const
{
	return block_at(x, y, z, allow_out_of_bounds).is_invisible();
}

bool ChunkMesher::block_is_invisible_not_none(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds) const
{
	const Block& block = block_at(x, y, z, allow_out_of_bounds);
	return block.is_invisible() && (block.type() != BlockType::none);
}
