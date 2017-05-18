#include "Base.hpp"

#include <stdexcept>
#include <string>

#include "Game.hpp"
#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/Color.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"

using std::string;

namespace Block {

Base::Base()
:
	type_(Enum::Type::none)
{
}

Base::Base(const Enum::Type type)
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
	const auto type1 = static_cast<Enum::Type_t>(type());
	const auto type2 = static_cast<Enum::Type_t>(that.type());
	if(type1 != type2)
	{
		throw std::runtime_error("can not copy " + std::to_string(type2) + " to " + std::to_string(type1));
	}
	return *this;
}

Enum::Type Base::type() const
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

fs::path Base::texture(const Enum::Face) const
{
	return {};
}

double Base::bounciness() const
{
	return 0;
}

glm::dvec4 Base::selection_color() const
{
	return {1, 1, 1, 1};
}

Graphics::Color Base::light_filter() const
{
	return {Graphics::Color::max};
}

Enum::VisibilityType Base::visibility_type() const
{
	return Enum::VisibilityType::opaque;
}

bool Base::is_opaque() const
{
	return visibility_type() == Enum::VisibilityType::opaque;
}

bool Base::is_translucent() const
{
	return visibility_type() == Enum::VisibilityType::translucent;
}

bool Base::is_invisible() const
{
	return visibility_type() == Enum::VisibilityType::invisible;
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
	Enum::Type t = (type_ == Enum::Type::none) ? Enum::Type::air : type_;
	Enum::TypeExternal te = Game::instance->block_registry.get_extid(t);
	i.set("", te);
}

void Base::load(Storage::InputInterface&)
{
	// type is set before loading
}

}
