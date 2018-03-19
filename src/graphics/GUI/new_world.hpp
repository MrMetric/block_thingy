#pragma once
#include "Base.hpp"

namespace block_thingy::graphics::gui {

class new_world : public Base
{
public:
	new_world(game&);

	std::string type() const override;
};

}
