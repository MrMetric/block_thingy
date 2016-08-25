#include "Block.hpp"

#include "BlockType.hpp"
#include "graphics/Color.hpp"

namespace Block {

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

Graphics::Color Block::color() const
{
	const auto m = Graphics::Color::max;
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wswitch"
	switch(type())
	#pragma clang diagnostic pop
	{
		case BlockType::light_test_red:		return { m,  0,  0};
		case BlockType::light_test_green:	return { 0,  m,  0};
		case BlockType::light_test_blue:	return { 0,  0,  m};
		case BlockType::light_test_yellow:	return { m,  m,  0};
		case BlockType::light_test_cyan:	return { 0,  m,  m};
		case BlockType::light_test_pink:	return { m,  0,  m};
		case BlockType::light_test_white:	return { m,  m,  m};
	}

	return {0, 0, 0};
}

double Block::bounciness() const
{
	if(type_ == BlockType::test)
	{
		return 1;
	}
	return 0;
}

BlockVisibilityType Block::visibility_type() const
{
	if(type_ == BlockType::none || type_ == BlockType::air)
	{
		return BlockVisibilityType::invisible;
	}
	if(type_ == BlockType::teleporter)
	{
		return BlockVisibilityType::translucent;
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
	return type_ != BlockType::air && type_ != BlockType::teleporter;
}

bool Block::is_selectable() const
{
	return type_ != BlockType::air;
}

} // namespace Block
