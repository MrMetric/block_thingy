#include "Light.hpp"

#include <glm/vec3.hpp>

#include "block/BlockType.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/Color.hpp"

using Graphics::Color;

namespace Block {

Light::Light()
	:
	Light(Color(glm::dvec3(1, 1, 1)))
{
}

Light::Light(const Color& color)
	:
	Base(BlockType::light),
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
