#pragma once
#include "Base.hpp"

#include <glm/vec2.hpp>

#include "fwd/Game.hpp"

namespace Graphics::GUI {

class Play : public Base
{
public:
	Play(Game&);

	std::string type() const override;

	void init() override;
	void close() override;
	void draw() override;

	void keypress(const Util::key_press&) override;
	void mousepress(const Util::mouse_press&) override;
	void mousemove(double x, double y) override;
	void joypress(int joystick, int button, bool pressed) override;
	void joymove(const glm::dvec2& motion) override;

private:
	void draw_gui() override;
	void draw_crosshair();
	void draw_debug_text();
};

}
