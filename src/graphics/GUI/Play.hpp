#pragma once
#include "Base.hpp"

#include <glm/vec2.hpp>

#include "fwd/Game.hpp"

namespace Graphics::GUI {

class Play : public Base
{
	public:
		Play(Game&);

		void init() override;
		void close() override;
		void draw() override;

		void keypress(int key, int scancode, int action, int mods) override;
		void mousepress(int button, int action, int mods) override;
		void mousemove(double x, double y) override;
		void joypress(int joystick, int button, bool pressed) override;
		void joymove(const glm::dvec2& motion) override;

	private:
		void draw_gui() override;
		void draw_crosshair();
		void draw_debug_text();
};

} // namespace Graphics::GUI
