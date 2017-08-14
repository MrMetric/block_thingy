#include "ChunkData.hpp"

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/Enum/Type.hpp"

template<>
ChunkData<std::shared_ptr<Block::Base>>::ChunkData()
:
	ChunkData(Game::instance->block_registry.get_default(Block::Enum::Type::air))
{
}
