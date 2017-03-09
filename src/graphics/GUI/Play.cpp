#include "Play.hpp"

#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "block/Base.hpp"
#include "fwd/block/BlockType.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "util/key_mods.hpp"

namespace Graphics::GUI {

Play::Play(Game& game)
:
	Base(game, WidgetContainerMode::rows)
{
}

void Play::init()
{
	glClearColor(0, 0, 0.1f, 0);
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	game.gfx.center_cursor();
}

void Play::close()
{
	game.console.run_line("open_gui pause");
}

void Play::draw()
{
	game.draw_world();
	if(game.gui.get() == this)
	{
		game.step_world();
	}
	Base::draw();
}

void Play::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
	game.keybinder.keypress(key, scancode, action, mods);
}

void Play::mousepress(const int button, const int action, const Util::key_mods mods)
{
	game.keybinder.mousepress(button, action, mods);
}

void Play::mousemove(const double x, const double y)
{
	game.camera.mousemove(x, y);
}

void Play::joypress(const int joystick, const int button, const bool pressed)
{
	game.keybinder.joypress(joystick, button, pressed);
}

void Play::joymove(const glm::dvec2& motion)
{
	const glm::dvec2 pos = motion * 32.0 + game.gfx.window_mid;
	game.camera.mousemove(pos.x, pos.y, true);
}

void Play::draw_gui()
{
	draw_crosshair();
	draw_debug_text();
}

void Play::draw_crosshair()
{
	const glm::dvec4 crosshair_color(1.0);

	// TODO: rectangle widget
	const double x = game.gfx.window_mid.x;
	const double y = game.gfx.window_mid.y;
	game.gfx.draw_rectangle({x - 1, y - 16}, {2, 32}, crosshair_color);
	game.gfx.draw_rectangle({x - 16, y - 1}, {32, 2}, crosshair_color);
}

void Play::draw_debug_text()
{
	std::ostringstream ss;
	ss << std::boolalpha;
	const glm::dvec3 pos = game.player.position();
	#define p(x) (x > 0 ? "+" : (x < 0 ? "" : " ")) << x
	ss << "x: " << p(pos.x) << "\n";
	ss << "y: " << p(pos.y) << "\n";
	ss << "z: " << p(pos.z) << "\n";
	#undef p
	ss << "noclip: " << game.player.get_noclip() << "\n";
	ss << "block type: "
		<< game.block_registry.get_strid(game.block_type)
		<< " (" << static_cast<block_type_id_t>(game.block_type) << ")"
		<< "\n";
	if(game.hovered_block != nullptr)
	{
		const Block::Base& hovered = game.world.get_block(game.hovered_block->pos);
		ss << "hovered: "
			<< hovered.name()
			<< " (" << static_cast<block_type_id_t>(hovered.type()) << ")"
			<< "\n";
	}
	game.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

} // namespace Graphics::GUI
