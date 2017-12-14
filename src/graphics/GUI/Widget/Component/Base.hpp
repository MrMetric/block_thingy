#pragma once

#include <glm/vec2.hpp>

namespace block_thingy::graphics::gui::widget::component {

class Base
{
public:
	virtual ~Base();

	virtual void draw(const glm::dvec2& w_position, const glm::dvec2& w_size) = 0;
};

}
