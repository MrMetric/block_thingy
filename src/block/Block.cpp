#include "Block.hpp"

#include <stdexcept>
#include <string>

#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"
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

Block::~Block()
{
}

Block::Block(const Block& that)
	:
	type_(that.type_)
{
	operator=(that);
}

void Block::operator=(const Block& that)
{
	const auto type1 = type_id();
	const auto type2 = that.type_id();
	if(type1 != type2)
	{
		throw std::runtime_error("can not copy " + std::to_string(type2) + " to " + std::to_string(type1));
	}
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
	return {0, 0, 0};
}

double Block::bounciness() const
{
	return 0;
}

BlockVisibilityType Block::visibility_type() const
{
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
	return true;
}

bool Block::is_selectable() const
{
	return true;
}

} // namespace Block
