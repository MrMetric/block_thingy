#pragma once
#include "Base.hpp"

#include <string>

#include "graphics/GUI/Widget/Component/Text.hpp"
#include "shim/propagate_const.hpp"

namespace block_thingy::graphics::gui::widget {

class Text : public Base
{
public:
	Text
	(
		Base* parent,
		const std::string& text = ""
	);
	~Text() override;

	std::string type() const override;

	void draw() override;

	void read_layout(const json&) override;

	std::string get_text() const;
	void set_text(const std::string&);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
