#pragma once
#include "Base.hpp"

#include "fwd/Game.hpp"

namespace Graphics::GUI {

class Pause : public Base
{
	public:
		Pause(Game&);

		std::string type() const override;

		void init() override;
		void draw() override;
};

}
