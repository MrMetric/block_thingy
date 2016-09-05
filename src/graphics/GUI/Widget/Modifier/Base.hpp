#pragma once

#include "fwd/graphics/GUI/Widget/Base.hpp"

namespace Graphics::GUI::Widget::Modifier {

class Base
{
	public:
		virtual ~Base();

		virtual void draw(const Widget::Base&) = 0;
};

} // namespace Graphics::GUI::Widget::Modifier
