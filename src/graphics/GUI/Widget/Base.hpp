#pragma once

#include <glm/vec2.hpp>

#include "fwd/graphics/GUI/WidgetContainer.hpp"

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

		virtual void draw() = 0;

		virtual void mousepress(int button, int action, int mods);
		virtual void mousemove(double x, double y);

		void update_container
		(
			const glm::dvec2& container_position,
			const glm::dvec2& container_size,
			const glm::dvec2& offset
		);

		glm::dvec2 get_size();
		glm::dvec2 get_origin();

	protected:
		WidgetContainer& owner;

		glm::dvec2 size;
		glm::dvec2 origin;
		glm::dvec2 real_position;
};

} // namespace Graphics::GUI::Widget
