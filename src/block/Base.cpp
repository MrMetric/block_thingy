#include "Base.hpp"

#include <stdexcept>
#include <string>

#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"
#include "graphics/Color.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"

namespace Block {

Base::Base() : type_(BlockType::none) {}

Base::Base(const block_type_id_t type_id)
	:
	type_(static_cast<BlockType>(type_id))
{
}

Base::Base(const BlockType type)
	:
	type_(type)
{
}

Base::~Base()
{
}

Base::Base(const Base& that)
	:
	type_(that.type_)
{
	operator=(that);
}

void Base::operator=(const Base& that)
{
	const auto type1 = type_id();
	const auto type2 = that.type_id();
	if(type1 != type2)
	{
		throw std::runtime_error("can not copy " + std::to_string(type2) + " to " + std::to_string(type1));
	}
}

block_type_id_t Base::type_id() const
{
	return static_cast<block_type_id_t>(type_);
}

BlockType Base::type() const
{
	return type_;
}

Graphics::Color Base::color() const
{
	return {0, 0, 0};
}

double Base::bounciness() const
{
	return 0;
}

BlockVisibilityType Base::visibility_type() const
{
	return BlockVisibilityType::opaque;
}

bool Base::is_opaque() const
{
	return visibility_type() == BlockVisibilityType::opaque;
}

bool Base::is_translucent() const
{
	return visibility_type() == BlockVisibilityType::translucent;
}

bool Base::is_invisible() const
{
	return visibility_type() == BlockVisibilityType::invisible;
}

bool Base::is_solid() const
{
	return true;
}

bool Base::is_selectable() const
{
	return true;
}

void Base::save(Storage::OutputInterface& i) const
{
	BlockType t = type_ != BlockType::none ? type_ : BlockType::air;
	i.set("t", t);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter" // i
void Base::load(Storage::InputInterface& i)
#pragma clang diagnostic pop
{
	// type is set before loading
}

} // namespace Block
