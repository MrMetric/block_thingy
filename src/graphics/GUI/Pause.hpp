#pragma once
#include "Base.hpp"

#include "fwd/Game.hpp"

namespace Graphics {
namespace GUI {

class Pause : public Base
{
	public:
		Pause(Game&);

		void init() override;
		void draw() override;

	private:
		void draw_gui() override;
};

} // namespace GUI
} // namespace Graphics
