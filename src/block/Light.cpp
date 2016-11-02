#include "Light.hpp"

#include <glm/vec3.hpp>

#include "Game.hpp"
#include "block/BlockType.hpp"
#include "graphics/GUI/Light.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/Color.hpp"

using Graphics::Color;

namespace Block {

Light::Light(BlockType t)
:
	Light(t, Color(glm::dvec3(1, 1, 1)))
{
}

Light::Light(BlockType t, const Color& color)
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

Color Light::color() const
{
	return color_;
}

void Light::color(const Color& c)
{
	Game::instance->world.sub_light(position);
	color_ = c;
	Game::instance->world.add_light(position, color_, true);
}

void Light::use_start()
{
	Game::instance->open_gui(std::make_unique<Graphics::GUI::Light>(*Game::instance, *this));
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
