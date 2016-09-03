#pragma once
#include "Base.hpp"

#include <string>

#include <glm/vec2.hpp>

namespace Graphics {
namespace GUI {
namespace Widget {

class Text : public Base
{
	public:
		Text
		(
			WidgetContainer& owner,
			const std::string& text
		);

		void draw() override;

	private:
		std::string text;
};

} // namespace Widget
} // namespace GUI
} // namespace Graphics

