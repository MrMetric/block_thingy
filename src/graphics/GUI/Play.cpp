#include "Play.hpp"

#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/io.hpp>

#include "Camera.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "Settings.hpp"
#include "block/Base.hpp"
#include "block/Enum/Type.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/key_mods.hpp"

using std::shared_ptr;
using std::string;

namespace Graphics::GUI {

Play::Play(Game& game)
:
	Base(game, "")
{
}

string Play::type() const
{
	return "Play";
}

void Play::init()
{
	glClearColor(0, 0, 0.1f, 0);
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	game.gfx.center_cursor();
}

void Play::close()
{
	Console::instance->run_line("open_gui Pause");
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

void Play::keypress(const Util::key_press& press)
{
	game.keybinder.keypress(press);
}

void Play::mousepress(const Util::mouse_press& press)
{
	game.keybinder.mousepress(press);
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
	if(Settings::get<bool>("show_HUD"))
	{
		draw_crosshair();
	}
	if(Settings::get<bool>("show_debug_info"))
	{
		draw_debug_text();
	}
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

	ss << "framerate: " << game.get_fps() << '\n';
	ss << "render distance: " << game.get_render_distance() << '\n';

	const glm::dvec3 pos = game.player.position();
	ss << glm::io::precision(4); // default is 3
	ss << glm::io::width(10); // default is 9 (1 + 4 + 1 + default precision)
	ss << "position: " << pos << "\n";

	std::ostringstream ss2;
	ss2 << glm::io::precision(0);
	ss2 << glm::io::width(5);
	const Position::BlockInWorld player_block_pos(pos);
	ss2 << "\tblock in world: "
		<< static_cast<Position::BlockInWorld::vec_type>(player_block_pos) << '\n';
	ss2 << "\tchunk in world: "
		<< static_cast<Position::ChunkInWorld::vec_type>(Position::ChunkInWorld(player_block_pos)) << '\n';
	ss2 << "\tblock in chunk: "
		<< static_cast<Position::BlockInChunk::vec_type>(Position::BlockInChunk(player_block_pos)) << '\n';
	ss << ss2.str();

	ss << "rotation: " << game.camera.rotation << "\n";
	ss << "ticks: " << game.world.get_ticks() << "\n";
	ss << "time: " << game.world.get_time() << "\n";
	ss << "noclip: " << game.player.get_noclip() << "\n";
	auto show_block = [](const Block::Base& block) -> string
	{
		std::ostringstream ss;
		ss << block.name() << " (" << block.type() << ")";
		return ss.str();
	};
	if(game.copied_block != nullptr)
	{
		ss << "copied block: " << show_block(*game.copied_block) << "\n";
	}
	if(game.hovered_block != nullptr)
	{
		const shared_ptr<Block::Base> hovered = game.world.get_block(game.hovered_block->pos);
		ss << "hovered: " << show_block(*hovered) << "\n";
		ss << "\tface: " << game.hovered_block->face() << '\n';
		ss << "\trotation: " << glm::io::width(2) << hovered->rotation() << '\n';
		ss << "\tlight: " << game.world.get_light(game.hovered_block->adjacent()) << '\n';
	}

	game.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

}
