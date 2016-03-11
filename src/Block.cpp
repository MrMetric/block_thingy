#include "Block.hpp"

#include "BlockType.hpp"

Block::Block() : type_(BlockType::none) {}

Block::Block(const block_type_id_t type_id)
	:
	type_(static_cast<BlockType>(type_id))
{
}

Block::Block(const BlockType type)
	:
	type_(type)
{
}

block_type_id_t Block::type_id() const
{
	return static_cast<block_type_id_t>(type_);
}

BlockType Block::type() const
{
	return type_;
}

bool Block::is_invisible() const
{
	return type_ == BlockType::none || type_ == BlockType::air;
}

bool Block::is_solid() const
{
	return type_ != BlockType::air;
}
