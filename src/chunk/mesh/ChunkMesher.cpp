#include "ChunkMesher.hpp"

#include <cstdint>

#include "../Chunk.hpp"
#include "../../Block.hpp"
#include "../../BlockType.hpp"
#include "../../Coords.hpp"
#include "../../World.hpp"

ChunkMesher::ChunkMesher(const Chunk& chunk)
	:
	chunk(chunk)
{
}

ChunkMesher::~ChunkMesher()
{
}

const Block ChunkMesher::block_at(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		if(!allow_out_of_bounds)
		{
			return Block(BlockType::none);
		}
		auto chunk_pos = chunk.get_position();
		int64_t bx = chunk_pos.x * CHUNK_SIZE + x;
		int64_t by = chunk_pos.y * CHUNK_SIZE + y;
		int64_t bz = chunk_pos.z * CHUNK_SIZE + z;
		return chunk.get_owner()->get_block(Position::BlockInWorld(bx, by, bz));
	}
	#define s(a) static_cast<BlockInChunk_type>(a)
	return chunk.get_block(s(x), s(y), s(z));
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
