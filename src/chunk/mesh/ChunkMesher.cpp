#include "ChunkMesher.hpp"

#include "../Chunk.hpp"
#include "../../World.hpp"

ChunkMesher::ChunkMesher(const Chunk& chunk)
	:
	chunk(chunk)
{
}

ChunkMesher::~ChunkMesher()
{
}

bool ChunkMesher::block_is_empty(int_fast16_t x, int_fast16_t y, int_fast16_t z) const
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		auto chunk_pos = chunk.get_position();
		int64_t bx = chunk_pos.x * CHUNK_SIZE + x;
		int64_t by = chunk_pos.y * CHUNK_SIZE + y;
		int64_t bz = chunk_pos.z * CHUNK_SIZE + z;
		return chunk.get_owner()->get_block(Position::BlockInWorld(bx, by, bz)).type() == 0;
	}
	return chunk.get_block(x, y, z).type() == 0;
}
