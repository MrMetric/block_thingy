#include "Play.hpp"

#include <sstream>

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "Player.hpp"

namespace Graphics {
namespace GUI {

Play::Play(Game& game)
	:
	Base(game)
{
}

void Play::init()
{
	glClearColor(0.0, 0.0, 0.1, 0.0);
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(game.gfx.window, game.gfx.window_size.x / 2, game.gfx.window_size.y / 2);
}

void Play::close()
{
	game.console.run_line("open_gui pause");
}

void Play::draw()
{
	if(game.gui.get() == this)
	{
		game.step_world();
	}
	game.draw_world();
	Base::draw();
}

void Play::keypress(const int key, const int scancode, const int action, const int mods)
{
	game.keybinder.keypress(key, scancode, action, mods);
}

void Play::mousepress(const int button, const int action, const int mods)
{
	game.keybinder.mousepress(button, action, mods);
}

void Play::mousemove(const double x, const double y)
{
	game.camera.mousemove(x, y);
}

void Play::draw_gui()
{
	draw_crosshair();
	draw_debug_text();
}

void Play::draw_crosshair()
{
	const glm::dvec4 crosshair_color(1.0);
	game.gfx.draw_rectangle({0.5, 0.5}, {0, 0}, {2, 32}, crosshair_color);
	game.gfx.draw_rectangle({0.5, 0.5}, {0, 0}, {32, 2}, crosshair_color);
}

void Play::draw_debug_text()
{
	std::ostringstream ss;
	const auto& pos = game.player.position;
	ss << "x: " << pos.x << "\n";
	ss << "y: " << pos.y << "\n";
	ss << "z: " << pos.z << "\n";
	game.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

} // namespace GUI
} // namespace Graphics
