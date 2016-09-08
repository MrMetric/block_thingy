#pragma once

#include "fwd/graphics/GUI/Widget/Base.hpp"

namespace Graphics::GUI::Widget::Component {

class Base
{
	public:
		virtual ~Base();

		virtual void draw(const Widget::Base&) = 0;
};

}
