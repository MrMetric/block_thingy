#pragma once
#include "Base.hpp"

#include <string>

namespace Graphics::GUI::Widget {

class Text : public Base
{
	public:
		Text
		(
			const std::string& text = ""
		);

		std::string type() const override;

		void draw() override;

		void read_layout(const json&) override;

	private:
		std::string text;
};

}
