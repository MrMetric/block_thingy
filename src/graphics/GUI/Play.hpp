#pragma once
#include "Base.hpp"

#include <glm/vec2.hpp>

#include "fwd/game.hpp"

namespace block_thingy::graphics::gui {

class Play : public Base
{
public:
	Play(game&);

	std::string type() const override;

	void switch_to() override;
	void switch_from() override;
	void close() override;
	void draw() override;

	void keypress(const input::key_press&) override;
	void mousepress(const input::mouse_press&) override;
	void mousemove(const glm::dvec2& position) override;
	void joymove(const glm::dvec2& offset) override;

protected:
	void draw_gui() override;

private:
	void draw_crosshair();
	void draw_debug_text();
};

}
