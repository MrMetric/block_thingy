#include "Base.hpp"

#include "graphics/GUI/Widget/Modifier/Base.hpp"

namespace Graphics::GUI::Widget {

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

void Base::draw()
{
	for(auto& m : modifiers)
	{
		m->draw(*this);
	}
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

glm::dvec2 Base::get_size() const
{
	return size;
}

glm::dvec2 Base::get_origin() const
{
	return origin;
}

glm::dvec2 Base::get_real_position() const
{
	return real_position;
}

void Base::add_modifier(std::shared_ptr<Modifier::Base> m)
{
	modifiers.emplace_back(m);
}

} // namespace Graphics::GUI::Widget
