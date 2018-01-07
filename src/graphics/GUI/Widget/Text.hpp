#pragma once
#include "Base.hpp"

#include <string>

namespace block_thingy::graphics::gui::widget {

class Text : public Base
{
public:
	Text
	(
		Base* parent,
		const std::string& text = ""
	);

	std::string type() const override;

	void draw() override;

	void read_layout(const json&) override;

private:
	std::string text;
};

}
