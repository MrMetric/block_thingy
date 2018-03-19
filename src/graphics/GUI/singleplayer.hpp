#pragma once
#include "Base.hpp"

namespace block_thingy::graphics::gui {

class singleplayer : public Base
{
public:
	singleplayer(game&);

	std::string type() const override;
};

}
