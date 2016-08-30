#include "Base.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

namespace Graphics {
namespace GUI {
namespace Widget {

Base::Base
(
	WidgetContainer& owner,
	const glm::dvec2& size,
	const glm::dvec2& origin
)
:
	owner(owner),
	size(size),
	origin(origin)
{
}

Base::~Base()
{
}

void Base::mousepress(const int button, const int action, const int mods)
{
}

void Base::mousemove(const double x, const double y)
{
}

void Base::update_container
(
	const glm::dvec2& container_position,
	const glm::dvec2& container_size,
	const glm::dvec2& offset
)
{
	real_position = container_position + glm::dvec2(0.5) * container_size - size * origin + offset;
}

glm::dvec2 Base::get_size()
{
	return size;
}

glm::dvec2 Base::get_origin()
{
	return origin;
}

} // namespace Widget
} // namespace GUI
} // namespace Graphics
