#include "ChunkMesher.hpp"

#include <stdint.h>

#include "World.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

const meshmap_key_t ChunkMesher::empty_key = { BlockType::none, Graphics::Color() };

ChunkMesher::ChunkMesher()
{
}

ChunkMesher::~ChunkMesher()
{
}

const Block::Block& ChunkMesher::block_at(const Chunk& chunk, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z, const bool allow_out_of_bounds)
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		if(!allow_out_of_bounds)
		{
			static const Block::Block none(BlockType::none);
			return none;
		}
		Position::BlockInWorld block_pos(chunk.get_position(), {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_block(block_pos);
	}
	#define s(a) static_cast<Position::BlockInChunk::value_type>(a)
	return chunk.get_block(s(x), s(y), s(z));
	#undef s
}

bool ChunkMesher::block_visible_from(const Chunk& chunk, const Block::Block& block, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z)
{
	const Block::Block& sibling = block_at(chunk, x, y, z);
	return
		   sibling.type() != BlockType::none
		&& !block.is_invisible() // this block is visible
		&& !sibling.is_opaque() // this block can be seen thru the adjacent block
		&& block.type() != sibling.type() // do not show sides inside of adjacent translucent blocks (of the same type)
	;
}

Graphics::Color ChunkMesher::light_at(const Chunk& chunk, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z)
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		Position::ChunkInWorld chunk_pos = chunk.get_position();
		Position::BlockInWorld block_pos(chunk_pos, {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_light(block_pos);
	}
	#define s(a) static_cast<Position::BlockInChunk::value_type>(a)
	return chunk.get_light({ s(x), s(y), s(z) });
	#undef s
}
