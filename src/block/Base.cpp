#include "Base.hpp"

#include <stdexcept>
#include <string>

#include "Game.hpp"
#include "block/BlockType.hpp"
#include "block/BlockVisibilityType.hpp"
#include "graphics/Color.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"

using std::string;

namespace Block {

Base::Base()
:
	type_(BlockType::none)
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

Base& Base::operator=(const Base& that)
{
	const auto type1 = static_cast<block_type_id_t>(type());
	const auto type2 = static_cast<block_type_id_t>(that.type());
	if(type1 != type2)
	{
		throw std::runtime_error("can not copy " + std::to_string(type2) + " to " + std::to_string(type1));
	}
	return *this;
}

BlockType Base::type() const
{
	return type_;
}

string Base::name() const
{
	return Game::instance->block_registry.get_strid(type());
}

Graphics::Color Base::color() const
{
	return {0, 0, 0};
}

double Base::bounciness() const
{
	return 0;
}

glm::dvec4 Base::selection_color() const
{
	return {1, 1, 1, 1};
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

bool Base::is_replaceable() const
{
	return false;
}

void Base::use_start()
{
}

void Base::save(Storage::OutputInterface& i) const
{
	BlockType t = (type_ == BlockType::none) ? BlockType::air : type_;
	BlockTypeExternal te = Game::instance->block_registry.get_extid(t);
	i.set("", te);
}

void Base::load(Storage::InputInterface&)
{
	// type is set before loading
}

}
