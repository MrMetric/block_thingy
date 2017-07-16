#include "Light.hpp"

#include <memory>
#include <sstream>

#include <glm/vec3.hpp>

#include "Game.hpp"
#include "World.hpp"
#include "block/Enum/Type.hpp"
#include "graphics/GUI/Light.hpp"
#include "shim/make_unique.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/Color.hpp"

using std::string;
using Graphics::Color;

namespace Block {

Light::Light(Enum::Type t)
:
	Light(t, Color(glm::dvec3(1, 1, 1)))
{
}

Light::Light(Enum::Type t, const Color& color)
:
	Base(t),
	color_(color)
{
}

Light& Light::operator=(const Base& block)
{
	Base::operator=(block);
	const Light* that = static_cast<const Light*>(&block);
	color_ = that->color_;
	return *this;
}

string Light::name() const
{
	std::ostringstream ss;
	ss << "Light " << color();
	return ss.str();
}

Color Light::color() const
{
	return color_;
}

void Light::color(const Color& c)
{
	color_ = c;
}

void Light::use_start(const Position::BlockInWorld& pos, const Enum::Face)
{
	Game::instance->open_gui(std::make_unique<Graphics::GUI::Light>(*Game::instance, *this, pos));
}

void Light::save(Storage::OutputInterface& i) const
{
	Base::save(i);
	i.set("color", color_);
}

void Light::load(Storage::InputInterface& i)
{
	Base::load(i);
	color_ = i.get<Color>("color");
}

}
