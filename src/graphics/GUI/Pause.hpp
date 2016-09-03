#pragma once
#include "Base.hpp"

#include "fwd/Game.hpp"

namespace Graphics::GUI {

class Pause : public Base
{
	public:
		Pause(Game&);

		void init() override;
		void draw() override;
};

} // namespace Graphics::GUI
