#include "ChunkData.hpp"

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"

using T = std::unique_ptr<Block::Base>;

template<>
ChunkData<T>::ChunkData()
:
	ChunkData(Game::instance->block_registry.make(BlockType::air))
{
}

template<>
T ChunkData<T>::T_copy(const T& block) const
{
	return Game::instance->block_registry.make(*block);
}

template<>
bool ChunkData<T>::T_equal(const T& a, const T& b) const
{
	// TODO: implement operator== for blocks
	return false;
}
