#pragma once
#include "Base.hpp"

namespace block_thingy::graphics::gui {

class Console : public Base
{
public:
	Console(game&);

	std::string type() const override;

	void draw() override;
};

}
