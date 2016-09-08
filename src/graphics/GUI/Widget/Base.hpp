#pragma once

#include <memory>
#include <vector>

#include <glm/vec2.hpp>

#include "fwd/graphics/GUI/WidgetContainer.hpp"
#include "fwd/graphics/GUI/Widget/Component/Base.hpp"

namespace Graphics::GUI::Widget {

class Base
{
	public:
		Base
		(
			WidgetContainer& owner,
			const glm::dvec2& size,
			const glm::dvec2& origin = {0.5, 0.5}
		);
		virtual ~Base();

		virtual void draw();

		virtual void keypress(int key, int scancode, int action, int mods);
		virtual void charpress(char32_t codepoint);
		virtual void mousepress(int button, int action, int mods);
		virtual void mousemove(double x, double y);

		void update_container
		(
			const glm::dvec2& container_position,
			const glm::dvec2& container_size,
			const glm::dvec2& offset
		);

		glm::dvec2 get_size() const;
		glm::dvec2 get_origin() const;
		glm::dvec2 get_real_position() const;

		WidgetContainer& owner;

		void add_modifier(std::shared_ptr<Component::Base>);

	protected:
		glm::dvec2 size;
		glm::dvec2 origin;
		glm::dvec2 real_position;

	private:
		std::vector<std::shared_ptr<Component::Base>> modifiers;
};

} // namespace Graphics::GUI::Widget
