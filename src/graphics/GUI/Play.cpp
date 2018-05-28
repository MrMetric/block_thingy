#include "Play.hpp"

#include <cmath>
#include <sstream>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtx/io.hpp>

#include "game.hpp"
#include "Gfx.hpp"
#include "language.hpp"
#include "Player.hpp"
#include "settings.hpp"
#include "block/enums/Face.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "graphics/camera.hpp"
#include "graphics/color.hpp"
#include "input/key_mods.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "util/grisu2.hpp"
#include "util/logger.hpp"

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
	return "play";
}

void Play::switch_to()
{
	Base::switch_to();

	glClearColor(0, 0, 0.01f, 1);
	glfwSetInputMode(g.gfx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	g.gfx.center_cursor();
}

void Play::switch_from()
{
	glfwSetInputMode(g.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Play::close()
{
	Console::instance->run_line("open_gui pause");
}

void Play::draw()
{
	g.draw_world();
	Base::draw();
}

void Play::keypress(const input::key_press& press)
{
	g.keybinder.keypress(press);
}

void Play::mousepress(const input::mouse_press& press)
{
	g.keybinder.mousepress(press);
}

void Play::mousemove(const glm::dvec2& position)
{
	g.camera.mousemove(position - g.gfx.window_mid);
	g.gfx.center_cursor();
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
	const double size = settings::get<double>("crosshair_size");
	const double thickness = settings::get<double>("crosshair_thickness");

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

	ss << "framerate: ";
	const auto fps = g.get_fps();
	if(fps >= 50)
	{
		ss << logger::format::green;
	}
	else if(fps >= 30)
	{
		ss << logger::format::yellow;
	}
	else
	{
		ss << logger::format::red;
	}
	ss << fps << logger::format::reset << '\n';
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

	const glm::dvec3 pos = g.player->position();
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
	ss << "velocity: " << g.player->velocity() << '\n';
	ss << "global ticks: " << g.get_global_ticks() << '\n';
	ss << "global time : " << g.get_global_time() << '\n';
	ss << "world ticks : " << g.world->get_ticks() << '\n';
	ss << "world time  : " << g.world->get_time() << '\n';
	ss << "world seed  : " << util::grisu2(g.world->get_seed()) << '\n';
	ss << "world name  : " << g.world->get_name() << '\n';
	ss << "noclip: " << g.player->get_noclip() << '\n';
	auto show_block = [&g=this->g](const block_t block) -> string
	{
		std::ostringstream ss;
		const auto name = g.world->block_manager.get_name(block);
		if(name != nullopt)
		{
			ss << language::get(*name) << ' ';
		}
		const auto strid = g.world->block_manager.get_strid(block);
		if(strid != nullopt)
		{
			ss << '[' << *strid << "] ";
		}
		ss << '{' << block.index << ", " << block.generation << '}';
		return ss.str();
	};
	if(g.player->copied_block != nullopt)
	{
		ss << "copied block: " << show_block(*g.player->copied_block) << '\n';
	}
	if(g.player->hovered_block != nullopt)
	{
		const block_t hovered = g.player->hovered_block->block;
		ss << "hovered: " << show_block(hovered) << '\n';
		ss << "\tface: " << g.player->hovered_block->face() << '\n';
		ss << "\trotation: " << glm::io::width(2) << g.world->block_manager.info.rotation(hovered) << '\n';
		ss << "\temitted light: " << g.world->block_manager.info.light(hovered) << '\n';
		ss << "\tlight: " << g.world->get_blocklight(g.player->hovered_block->adjacent()) << '\n';
	}

	g.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

}
