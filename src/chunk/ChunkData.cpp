#include "ChunkData.hpp"

#include "Game.hpp"
#include "block/Base.hpp"

template<>
ChunkData<std::unique_ptr<Block::Base>>::ChunkData()
{
	fill(Game::instance->block_registry.make(BlockType::air));
}

template<>
void ChunkData<std::unique_ptr<Block::Base>>::fill(const std::unique_ptr<Block::Base>& block)
{
	std::generate(blocks.begin(), blocks.end(), [&block]()
	{
		return Game::instance->block_registry.make(*block);
	});
}
