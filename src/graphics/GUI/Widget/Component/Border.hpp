#pragma once
#include "Base.hpp"

#include <glm/vec4.hpp>

namespace Graphics::GUI::Widget::Component {

class Border : public Base
{
	public:
		Border
		(
			double size,
			const glm::dvec4& color
		);

		void draw(const Widget::Base&) override;

	private:
		double size;
		glm::dvec4 color;
};

}
