#pragma once
#include "Base.hpp"

#include "fwd/game.hpp"

namespace block_thingy::graphics::gui {

class Pause : public Base
{
public:
	Pause(game&);

	std::string type() const override;

	void draw() override;
};

}
