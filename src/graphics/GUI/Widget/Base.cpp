#include "Base.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

namespace Graphics {
namespace GUI {
namespace Widget {

Base::Base(WidgetContainer& owner, const glm::dvec2& position, const glm::dvec2& offset, const glm::dvec2& size, const glm::dvec2& origin)
	:
	owner(owner),
	position(position),
	offset(offset),
	size(size),
	origin(origin)
{
}

Base::~Base()
{
}

void Base::update_container(const glm::dvec2& container_position, const glm::dvec2& container_size)
{
	real_position = container_position + position * container_size - size * origin + offset;
}

} // namespace Widget
} // namespace GUI
} // namespace Graphics
