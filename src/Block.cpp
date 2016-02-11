#include "Block.hpp"

Block::Block() : type_(BlockType::none) {}

Block::Block(block_id_type type_id)
	:
	type_(static_cast<BlockType>(type_id))
{
}

Block::Block(BlockType type)
	:
	type_(type)
{
}

block_id_type Block::type_id() const
{
	return static_cast<block_id_type>(type_);
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
	return type_ != BlockType::none && type_ != BlockType::air;
}
