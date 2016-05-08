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

BlockVisibilityType Block::visibility_type() const
{
	if(type_ == BlockType::none || type_ == BlockType::air)
	{
		return BlockVisibilityType::invisible;
	}
	return BlockVisibilityType::opaque;
}

bool Block::is_opaque() const
{
	return visibility_type() == BlockVisibilityType::opaque;
}

bool Block::is_translucent() const
{
	return visibility_type() == BlockVisibilityType::translucent;
}

bool Block::is_invisible() const
{
	return visibility_type() == BlockVisibilityType::invisible;
}

bool Block::is_solid() const
{
	return type_ != BlockType::air;
}

bool Block::is_selectable() const
{
	return type_ != BlockType::air;
}
