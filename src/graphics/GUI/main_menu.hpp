#pragma once
#include "Base.hpp"

namespace block_thingy::graphics::gui {

class main_menu : public Base
{
public:
	main_menu(game&);

	std::string type() const override;

	void switch_to() override;
};

}
