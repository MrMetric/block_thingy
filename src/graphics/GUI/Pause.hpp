#pragma once
#include "Base.hpp"

#include "fwd/Game.hpp"

namespace block_thingy::graphics::gui {

class Pause : public Base
{
public:
	Pause(game&);

	std::string type() const override;

	void init() override;
	void draw() override;
};

}
