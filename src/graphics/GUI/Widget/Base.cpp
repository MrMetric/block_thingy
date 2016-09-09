#include "Base.hpp"

#include "graphics/GUI/Widget/Component/Base.hpp"
#include "util/key_mods.hpp"

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

void Base::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
}

void Base::charpress(const char32_t codepoint, const Util::key_mods mods)
{
}

void Base::mousepress(const int button, const int action, const Util::key_mods mods)
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

void Base::add_modifier(std::shared_ptr<Component::Base> m)
{
	modifiers.emplace_back(m);
}

} // namespace Graphics::GUI::Widget
