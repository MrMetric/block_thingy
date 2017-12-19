#include "Play.hpp"

#include <cmath>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/io.hpp>

#include "camera.hpp"
#include "game.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "settings.hpp"
#include "block/base.hpp"
#include "block/enums/Face.hpp"
#include "block/enums/type.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "graphics/color.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "util/key_mods.hpp"

using std::nullopt;
using std::shared_ptr;
using std::string;

namespace block_thingy::graphics::gui {

Play::Play(game& g)
:
	Base(g, "")
{
}

string Play::type() const
{
	return "Play";
}

void Play::init()
{
	glClearColor(0, 0, 0.1f, 0);
	glfwSetInputMode(g.gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	g.gfx.center_cursor();
}

void Play::close()
{
	Console::instance->run_line("open_gui Pause");
}

void Play::draw()
{
	if(g.gui.get() == this)
	{
		g.step_world();
	}
	g.draw_world();
	Base::draw();
}

void Play::keypress(const util::key_press& press)
{
	g.keybinder.keypress(press);
}

void Play::mousepress(const util::mouse_press& press)
{
	g.keybinder.mousepress(press);
}

void Play::mousemove(const glm::dvec2& position)
{
	g.camera.mousemove(position - g.gfx.window_mid);
	g.gfx.center_cursor();
}

void Play::joypress(const int joystick, const int button, const bool pressed)
{
	g.keybinder.joypress(joystick, button, pressed);
}

void Play::joymove(const glm::dvec2& offset)
{
	g.camera.joymove(offset);
}

void Play::draw_gui()
{
	if(settings::get<bool>("show_HUD"))
	{
		draw_crosshair();
	}
	if(settings::get<bool>("show_debug_info"))
	{
		draw_debug_text();
	}
}

void Play::draw_crosshair()
{
	const glm::dvec4 color = settings::get<glm::dvec4>("crosshair_color");
	const double size = settings::get<int64_t>("crosshair_size");
	const double thickness = settings::get<int64_t>("crosshair_thickness");

	// TODO: rectangle widget
	const double x = g.gfx.window_mid.x;
	const double y = g.gfx.window_mid.y;
	const double t2 = thickness / 2;
	const double s2 = size / 2;
	g.gfx.draw_rectangle({x - t2, y - s2}, {thickness, size}, color);
	g.gfx.draw_rectangle({x - s2, y - t2}, {s2 - t2, thickness}, color);
	g.gfx.draw_rectangle({x + t2, y - t2}, {s2 - t2, thickness}, color);
}

void Play::draw_debug_text()
{
	std::ostringstream ss;
	ss << std::boolalpha;
	ss << glm::io::precision(4); // default is 3
	ss << glm::io::width(10); // default is 9 (1 + 4 + 1 + default precision)

	ss << "framerate: " << g.get_fps() << '\n';
	ss << "camera position: " << g.gfx.graphical_position << '\n';

	{
		ss << "render distance: " << settings::get<int64_t>("render_distance") << '\n';
		const auto [total, drawn] = g.get_draw_stats();
		ss << "\tchunks in range: " << total << '\n';
		ss << "\tchunks drawn   : " << drawn;
		if(total != 0)
		{
			const double percent = std::round(static_cast<double>(drawn * 100 * 100) / total) / 100;
			ss << " (" << percent << "%)";
		}
		ss << '\n';
	}

	ss << "field of view: " << settings::get<double>("fov") << '\n';
	ss << "projection type: " << settings::get<string>("projection_type") << '\n';

	const glm::dvec3 pos = g.player.position();
	ss << "position: " << pos << '\n';

	std::ostringstream ss2;
	ss2 << glm::io::precision(0);
	ss2 << glm::io::width(5);
	const position::block_in_world player_block_pos(pos);
	ss2 << "\tblock in world: "
		<< static_cast<position::block_in_world::vec_type>(player_block_pos) << '\n';
	ss2 << "\tchunk in world: "
		<< static_cast<position::chunk_in_world::vec_type>(position::chunk_in_world(player_block_pos)) << '\n';
	ss2 << "\tblock in chunk: "
		<< static_cast<position::block_in_chunk::vec_type>(position::block_in_chunk(player_block_pos)) << '\n';
	ss << ss2.str();

	ss << "rotation: " << g.camera.rotation << '\n';
	ss << "ticks: " << g.world.get_ticks() << '\n';
	ss << "time: " << g.world.get_time() << '\n';
	ss << "noclip: " << g.player.get_noclip() << '\n';
	auto show_block = [](const block::base& block) -> string
	{
		std::ostringstream ss;
		ss << block.name() << " (" << block.type() << ')';
		return ss.str();
	};
	if(g.copied_block != nullptr)
	{
		ss << "copied block: " << show_block(*g.copied_block) << '\n';
	}
	if(g.hovered_block != nullopt)
	{
		const shared_ptr<block::base> hovered = g.world.get_block(g.hovered_block->pos);
		ss << "hovered: " << show_block(*hovered) << '\n';
		ss << "\tface: " << g.hovered_block->face() << '\n';
		ss << "\trotation: " << glm::io::width(2) << hovered->rotation() << '\n';
		ss << "\temitted light: " << hovered->light() << '\n';
		ss << "\tlight: " << g.world.get_blocklight(g.hovered_block->adjacent()) << '\n';
	}

	g.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

}
