#include "Base.hpp"

#include <stdint.h>

#include "Game.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

namespace Mesher {

const meshmap_key_t Base::empty_key = { BlockType::none, Graphics::Color() };

Base::Base()
{
}

Base::~Base()
{
}

const Block::Base& Base::block_at(const Chunk& chunk, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z, const bool allow_out_of_bounds)
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		if(!allow_out_of_bounds)
		{
			static const std::unique_ptr<Block::Base> none = Game::instance->block_registry.make(BlockType::none);
			return *none;
		}
		Position::BlockInWorld block_pos(chunk.get_position(), {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_block(block_pos);
	}
	if(chunk.solid_block != nullptr)
	{
		return *chunk.solid_block;
	}
	#define s(a) static_cast<Position::BlockInChunk::value_type>(a)
	return chunk.get_block(s(x), s(y), s(z));
	#undef s
}

bool Base::block_visible_from(const Chunk& chunk, const Block::Base& block, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z)
{
	const Block::Base& sibling = block_at(chunk, x, y, z);
	return
		   sibling.type() != BlockType::none
		&& !block.is_invisible() // this block is visible
		&& !sibling.is_opaque() // this block can be seen thru the adjacent block
		&& block.type() != sibling.type() // do not show sides inside of adjacent translucent blocks (of the same type)
	;
}

Graphics::Color Base::light_at(const Chunk& chunk, const int_fast16_t x, const int_fast16_t y, const int_fast16_t z)
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

}
