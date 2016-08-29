#include "Light.hpp"

#include <glm/vec3.hpp>

#include "block/BlockType.hpp"

namespace Block {

using Graphics::Color;

Light::Light()
	:
	Light(Color(glm::dvec3(1, 1, 1)))
{
}

Light::Light(const Color& color)
	:
	Block(BlockType::light),
	color_(color)
{
}

void Light::operator=(const Block& block)
{
	Block::operator=(block);
	const Light* that = dynamic_cast<const Light*>(&block);
	color_ = that->color_;
}

Color Light::color() const
{
	return color_;
}

}
