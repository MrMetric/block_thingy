#include "game.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>

#include "fps_manager.hpp"
#include "Player.hpp"
#include "resource_manager.hpp"
#include "settings.hpp"
#include "block/base.hpp"
#include "block/enums/type.hpp"
#include "chunk/Mesher/Greedy.hpp"
#include "chunk/Mesher/Simple.hpp"
#include "chunk/Mesher/Simple2.hpp"
#include "console/Command.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/image.hpp"
#include "graphics/render_world.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/Console.hpp"
#include "graphics/GUI/Pause.hpp"
#include "graphics/GUI/Play.hpp"
#include "graphics/opengl/push_state.hpp"
#include "physics/ray.hpp"
#include "physics/raycast_hit.hpp"
#include "physics/raycast_util.hpp"
#include "plugin/PluginManager.hpp"
#include "position/block_in_chunk.hpp"
#include "position/block_in_world.hpp"
#include "position/chunk_in_world.hpp"
#include "util/demangled_name.hpp"
#include "util/filesystem.hpp"
#include "util/key_press.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::nullopt;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace block_thingy {

namespace detail {

gui_maker_base::~gui_maker_base()
{
}

}

game* game::instance = nullptr;

struct game::impl
{
	impl(game& g)
	:
		g(g),
		delta_time(0),
		fps(999),
		just_opened_gui(false),
		last_key(0),
		last_key_scancode(0),
		consume_key_release(0),
		consume_key_release_scancode(0)
	{
	}

	game& g;

	double delta_time;
	fps_manager fps;
	std::tuple<uint64_t, uint64_t> draw_stats;

	void find_hovered_block();

	std::vector<Command> commands;
	void add_commands();

	std::unordered_map<string, unique_ptr<detail::gui_maker_base>> gui_makers;
	graphics::gui::Base* root_gui;
	unique_ptr<graphics::gui::Base> temp_gui;

	bool just_opened_gui;
	int last_key;
	int last_key_scancode;
	int consume_key_release;
	int consume_key_release_scancode;
};

static unique_ptr<mesher::base> make_mesher(const string& name)
{
	unique_ptr<mesher::base> mesher;
	if(name == "greedy")
	{
		return std::make_unique<mesher::greedy>();
	}
	else if(name == "simple")
	{
		return std::make_unique<mesher::simple>();
	}
	else if(name == "simple2")
	{
		return std::make_unique<mesher::simple2>();
	}
	LOG(ERROR) << "No such mesher: " << name << '\n';
	return make_mesher("simple2");
}

game::game()
:
	set_instance(this),
	world("worlds/test", block_registry, make_mesher(settings::get<string>("mesher"))),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	keybinder(*Console::instance),
	pImpl(std::make_unique<impl>(*this))
{
	resource_manager.load_blocks(*this);

	if(block_registry.get_extid_map().empty())
	{
		// this should be done when starting a new world, but the design does not work that way yet
		block_registry.reset_extid_map();
	}

	register_gui<graphics::gui::Console>("console");
	register_gui<graphics::gui::Pause>("pause");
	register_gui<graphics::gui::Play>("play");

	gui = make_gui("play");
	pImpl->root_gui = gui.get();
	gui->init();

	gfx.hook_events(event_manager);

	pImpl->add_commands();
	Console::instance->run_line("exec binds");

	update_framebuffer_size(gfx.window_size);

	glfwSetJoystickCallback([](const int joystick, const int event)
	{
		if(joystick == GLFW_JOYSTICK_1 && event == GLFW_DISCONNECTED)
		{
			game::instance->player.set_analog_motion({ 0, 0 });
		}
	});
	glfwSetWindowFocusCallback(gfx.window, []([[maybe_unused]] GLFWwindow* window, const int focused)
	{
		assert(window == Gfx::instance->window);
		if(!focused)
		{
			if(game::instance->gui->type() == "play")
			{
				Console::instance->run_line("open_gui pause");
			}
		}
		// check if pause because a focus event is sent when the game starts
		else if(game::instance->gui->type() == "pause")
		{
			// when the game is paused after losing focus, the cursor stays hidden
			// GLFW ignores setting the cursor to its current state, so re-hide it first
			glfwSetInputMode(game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetInputMode(game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	});

	event_manager.add_handler(EventType::change_setting, [&game=*this](const Event& event)
	{
		const auto& e = static_cast<const Event_change_setting&>(event);

		if(e.name == "mesher")
		{
			const string name = *e.new_value.get<string>();
			game.world.set_mesher(make_mesher(name));
		}
	});

	PluginManager::instance->init_plugins(*this);

	copied_block = block_registry.get_default("light");
}

game::~game()
{
	settings::save();
}

void game::draw()
{
	// TODO: use double when available
	const float global_time = static_cast<float>(world.get_time());
	resource_manager.foreach_shader_program([global_time](resource<graphics::opengl::shader_program> r)
	{
		r->uniform("global_time", global_time);
	});

	pImpl->draw_stats = {0, 0};

	if(pImpl->temp_gui != nullptr)
	{
		pImpl->temp_gui = nullptr;
	}

	resource_manager.check_updates();

	glViewport(0, 0, gfx.window_size.x, gfx.window_size.y);

	glBindFramebuffer(GL_FRAMEBUFFER, gfx.screen_rt.frame_buffer.get_name());
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	gui->draw();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gfx.buf_rt.frame_buffer.get_name());
	// GL_READ_FRAMEBUFFER is screen_rt.frame_buffer
	glBlitFramebuffer
	(
		0, 0, gfx.window_size.x, gfx.window_size.y,
		0, 0, gfx.window_size.x, gfx.window_size.y,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(gfx.buf_rt.frame_texture.type, gfx.buf_rt.frame_texture.get_name());
	gfx.screen_shader->uniform("time", static_cast<float>(world.get_time()));
	gfx.screen_shader->use();
	gfx.quad_vao.draw(GL_TRIANGLES, 0, 6);

	// TODO: it might be faster to listen for the change event and set a private bool instead of getting the value every frame
	if(settings::get<string>("screen_shader") != "default")
	{
		glViewport(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
		glScissor(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		// TODO: split stepping and drawing to allow using gui->draw() here
		draw_world();
	}


	glfwSwapBuffers(gfx.window);

	glfwPollEvents();
	pImpl->just_opened_gui = false;

	if(glfwJoystickPresent(GLFW_JOYSTICK_1))
	{
		int count;

		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		for(int i = 0; i < count; ++i)
		{
			joypress(1, i, buttons[i] != 0);
		}

		auto fix_axis = [](const float axis) -> float
		{
			return (std::abs(axis) < 0.1f) ? 0 : axis;
		};

		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		assert(count % 2 == 0);

		assert(count == 8); // assume XInput
		/* layout on my computer:
		stick L x
		stick L y
		LT (PlayStation's L2)
		stick R x
		stick R y
		RT (PlayStation's R2)
		d-pad x
		d-pad y

		this is different on my other computer
		TODO: find out how to always have the right values
		*/

		glm::dvec2 stickL(fix_axis(axes[0]), fix_axis(axes[1]));
		glm::dvec2 stickR(fix_axis(axes[3]), fix_axis(axes[4]));

		player.set_analog_motion(stickL);
		joymove(stickR);
	}

	pImpl->delta_time = pImpl->fps.enforce_fps();
}

void game::step_world()
{
	player.rotation = camera.rotation;
	world.step(pImpl->delta_time);
	pImpl->find_hovered_block();
}

void game::draw_world()
{
	draw_world(camera.position, camera.rotation, gfx.projection_matrix);
}

void game::draw_world
(
	const glm::dvec3& cam_position,
	const glm::dvec3& cam_rotation,
	const glm::dmat4& projection_matrix
)
{
	draw_world(cam_position, Gfx::make_rotation_matrix(cam_rotation), projection_matrix);
}

void game::draw_world
(
	const glm::dvec3& cam_position,
	const glm::dmat4& cam_rotation,
	const glm::dmat4& projection_matrix
)
{
	const bool wireframe = settings::get<bool>("wireframe");
	graphics::opengl::push_state<GLenum, GL_POLYGON_MODE> _polygon_mode(wireframe ? GL_LINE : GL_FILL);

	gfx.set_camera_view(cam_position, cam_rotation, projection_matrix);
	position::block_in_world render_origin(cam_position);
	const std::tuple<uint64_t, uint64_t> draw_stats = graphics::draw_world
	(
		world,
		resource_manager,
		gfx.vp_matrix,
		render_origin,
		static_cast<uint64_t>(settings::get<int64_t>("render_distance"))
	);
	pImpl->draw_stats =
	{
		std::get<0>(pImpl->draw_stats) + std::get<0>(draw_stats),
		std::get<1>(pImpl->draw_stats) + std::get<1>(draw_stats),
	};

	if(hovered_block != nullopt && settings::get<bool>("show_HUD"))
	{
		const glm::dvec4 color = world.get_block(hovered_block->pos)->selection_color();
		gfx.draw_block_outline(hovered_block->pos, color);
	}
}

unique_ptr<graphics::gui::Base> game::make_gui(const string& type)
{
	const auto i = pImpl->gui_makers.find(type);
	if(i == pImpl->gui_makers.cend())
	{
		return nullptr;
	}
	return i->second->make(*this);
}

void game::open_gui(unique_ptr<graphics::gui::Base> gui)
{
	if(gui == nullptr)
	{
		LOG(WARN) << "Tried to open a null GUI\n";
		return;
	}
	gui->init();
	gui->parent = std::move(this->gui);
	this->gui = std::move(gui);

	double x, y;
	glfwGetCursorPos(gfx.window, &x, &y);
	this->gui->mousemove(glm::dvec2(x, y));

	pImpl->just_opened_gui = true;
}

void game::close_gui()
{
	if(gui->parent == nullptr)
	{
		if(gui.get() != pImpl->root_gui)
		{
			LOG(BUG) << "attempted to close a GUI which erroneously has no parent (type: " << gui->type() << ")\n";
		}
		return;
	}

	// code may be running in the GUI, such as from clicking the Resume button in the pause menu
	// after immediate destructing, graphics::gui::widget::Container will continue its mousepress loop
	// this invokes undefined behavior, so temp_gui keeps it for the rest of the frame
	pImpl->temp_gui = std::move(gui);
	gui = std::move(pImpl->temp_gui->parent);
	gui->init();
}

void game::quit()
{
	glfwSetWindowShouldClose(gfx.window, GL_TRUE);
}

void game::screenshot(fs::path path) const
{
	if(fs::is_directory("screenshots") || fs::create_directory("screenshots"))
	{
		path = "screenshots" / path;
	}
	// TODO: check file existence
	LOG(INFO) << "saving screenshot to " << path.u8string() << '\n';
	const auto width = gfx.window_size.x;
	const auto height = gfx.window_size.y;
	std::vector<uint8_t> pixels(4 * width * height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	graphics::image(width, height, std::move(pixels)).write(path);
}

double game::get_fps() const
{
	return pImpl->fps.get_fps();
}

std::tuple<uint64_t, uint64_t> game::get_draw_stats() const
{
	return pImpl->draw_stats;
}

void game::update_framebuffer_size(const window_size_t& window_size)
{
	gfx.update_framebuffer_size(window_size);
	event_manager.do_event(Event_window_size_change(window_size));
}

void game::keypress(const util::key_press& press)
{
	if(pImpl->consume_key_release == press.key && pImpl->consume_key_release_scancode == press.scancode)
	{
		pImpl->consume_key_release = 0;
		pImpl->consume_key_release_scancode = 0;
		if(press.action == GLFW_RELEASE)
		{
			return;
		}
	}
	pImpl->last_key = press.key;
	pImpl->last_key_scancode = press.scancode;
	gui->keypress(press);
}

void game::charpress(const util::char_press& press)
{
	if(pImpl->just_opened_gui)
	{
		// when a GUI is opened with a keybind, the GUI should not receive the key release event
		pImpl->consume_key_release = pImpl->last_key;
		pImpl->consume_key_release_scancode = pImpl->last_key_scancode;
		return;
	}
	gui->charpress(press);
}

void game::mousepress(const util::mouse_press& press)
{
	gui->mousepress(press);
}

void game::mousemove(const glm::dvec2& position)
{
	gui->mousemove(position);
}

void game::joypress(const int joystick, const int button, const bool pressed)
{
	gui->joypress(joystick, button, pressed);
}

void game::joymove(const glm::dvec2& offset)
{
	gui->joymove(offset);
}

void game::register_gui(const string& type, unique_ptr<detail::gui_maker_base> maker)
{
	pImpl->gui_makers.emplace(type, std::move(maker));
}

void game::impl::find_hovered_block()
{
	physics::ray ray = physics::screen_pos_to_world_ray
	(
		g.gfx.window_mid,
		g.gfx.window_size,
		g.gfx.view_matrix_graphical,
		g.gfx.projection_matrix
	);

	glm::dvec3 offset = g.gfx.physical_position - g.gfx.graphical_position;
	ray.origin += offset;
	g.hovered_block = physics::raycast
	(
		g.world,
		ray,
		g.player.reach_distance
	);
}

void game::impl::add_commands()
{
	// [[maybe_unused]] does not work with lambda captures
	// preprocessor directives do not work in #define, so I can not use #pragma clang diagnostic ...
	#define COMMAND(name) commands.emplace_back(*Console::instance, name, \
	[ \
		&g=g, \
		&player=g.player \
	] \
	( \
		[[maybe_unused]] const std::vector<string>& args \
	)

	COMMAND("save")
	{
		g.world.save();
	});
	COMMAND("quit")
	{
		g.quit();
	});

	COMMAND("break_block")
	{
		if(g.hovered_block == nullopt)
		{
			return;
		}

		const position::block_in_world pos = g.hovered_block->pos;
		shared_ptr<block::base> block = g.world.get_block(pos);
		if(block->type() != block::enums::type::none) // TODO: breakability check
		{
			g.world.set_block(pos, g.block_registry.get_default(block::enums::type::air), false);
		}
	});
	COMMAND("place_block")
	{
		if(g.hovered_block == nullopt || g.copied_block == nullptr)
		{
			return;
		}

		shared_ptr<block::base> block = g.copied_block;
		const position::block_in_world pos = g.hovered_block->adjacent();
		if(g.world.get_block(pos)->is_replaceable_by(*block)
		&& (player.can_place_block_at(pos) || !block->is_solid()))
		{
			g.world.set_block(pos, block, false);
		}
	});
	COMMAND("copy_block")
	{
		if(g.hovered_block != nullopt)
		{
			g.copied_block = g.world.get_block(g.hovered_block->pos);
		}
	});
	COMMAND("set_block")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: set_block <string: strid>\n";
			return;
		}
		try
		{
			g.copied_block = g.block_registry.get_default(args[0]);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << e.what() << '\n';
		}
	});

	// TODO: less copy/paste
	COMMAND("+forward")
	{
		player.move_forward(true);
	});
	COMMAND("-forward")
	{
		player.move_forward(false);
	});
	COMMAND("+backward")
	{
		player.move_backward(true);
	});
	COMMAND("-backward")
	{
		player.move_backward(false);
	});
	COMMAND("+left")
	{
		player.move_left(true);
	});
	COMMAND("-left")
	{
		player.move_left(false);
	});
	COMMAND("+right")
	{
		player.move_right(true);
	});
	COMMAND("-right")
	{
		player.move_right(false);
	});
	COMMAND("jump")
	{
		player.jump();
	});
	COMMAND("+use")
	{
		if(g.hovered_block != nullopt)
		{
			g.world.get_block(g.hovered_block->pos)->use_start
			(
				g,
				g.world,
				player,
				g.hovered_block->pos,
				g.hovered_block->face()
			);
		}
	});
	COMMAND("-use")
	{
		// TODO
	});
	COMMAND("+sprint")
	{
		player.go_faster(true);
	});
	COMMAND("-sprint")
	{
		player.go_faster(false);
	});

	COMMAND("noclip")
	{
		player.toggle_noclip();
	});
	COMMAND("respawn")
	{
		player.respawn();
		LOG(INFO) << "respawned at " << glm::to_string(player.position()) << '\n';
	});

	COMMAND("save_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: save_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		std::ofstream streem(save_name);
		streem.precision(std::numeric_limits<double>::max_digits10);

		const glm::dvec3 pos = player.position();
		streem << pos.x << ' ' << pos.y << ' ' << pos.z << '\n';

		const glm::dvec3 rot = player.rotation();
		streem << rot.x << ' ' << rot.y << ' ' << rot.z << '\n';

		streem.flush();
		LOG(INFO) << "saved position and rotation to " << save_name << '\n';
	});
	COMMAND("load_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: load_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		std::ifstream streem(save_name);

		glm::dvec3 pos;
		streem >> pos.x;
		streem >> pos.y;
		streem >> pos.z;
		player.position = pos;
		LOG(INFO) << "set position to " << glm::to_string(pos) << '\n';

		glm::dvec3 rot;
		streem >> rot.x;
		streem >> rot.y;
		streem >> rot.z;
		player.rotation = rot;
		LOG(INFO) << "set rotation to " << glm::to_string(rot) << '\n';
	});

	COMMAND("cam.rot")
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: cam.rot x|y|z <number: degrees>\n";
			return;
		}
		const string part = args[0];
		const double value = std::stod(args[1]);
		if(part == "x")
		{
			g.camera.rotation.x += value;
		}
		else if(part == "y")
		{
			g.camera.rotation.y += value;
		}
		else if(part == "z")
		{
			g.camera.rotation.z += value;
		}
		else
		{
			LOG(ERROR) << "component name must be x, y, or z\n";
			return;
		}
		LOG(INFO) << "camera rotation: " << glm::to_string(g.camera.rotation) << '\n';
	});

	COMMAND("screenshot")
	{
		string filename;
		if(args.size() == 0)
		{
			filename = util::datetime() + ".png";
		}
		else if(args.size() == 1)
		{
			filename = args[0];
		}
		else
		{
			LOG(ERROR) << "Usage: screenshot [string: filename]\n";
			return;
		}
		try
		{
			g.screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error saving screenshot: " << e.what() << '\n';
		}
	});

	COMMAND("reach_distance++")
	{
		player.reach_distance += 1;
		LOG(INFO) << "reach distance: " << player.reach_distance << '\n';
	});
	COMMAND("reach_distance--")
	{
		player.reach_distance -= 1;
		LOG(INFO) << "reach distance: " << player.reach_distance << '\n';
	});

	COMMAND("nazi")
	{
		if(g.hovered_block == nullopt || g.copied_block == nullptr)
		{
			return;
		}

		const position::block_in_world start_pos = g.hovered_block->adjacent();
		const position::block_in_world::value_type ysize = 9;
		const position::block_in_world::value_type xsize = 9;
		const block::enums::type_t i = static_cast<block::enums::type_t>(g.copied_block->type()); // TODO: use copied_block instance
		const block::enums::type_t nazi[ysize][xsize]
		{
			{ i, 1, 1, 1, i, i, i, i, i, },
			{ i, 1, 1, 1, i, 1, 1, 1, 1, },
			{ i, 1, 1, 1, i, 1, 1, 1, 1, },
			{ i, 1, 1, 1, i, 1, 1, 1, 1, },
			{ i, i, i, i, i, i, i, i, i, },
			{ 1, 1, 1, 1, i, 1, 1, 1, i, },
			{ 1, 1, 1, 1, i, 1, 1, 1, i, },
			{ 1, 1, 1, 1, i, 1, 1, 1, i, },
			{ i, i, i, i, i, 1, 1, 1, i, },
		};
		position::block_in_world pos;
		for(pos.x = 0; pos.x < xsize; ++pos.x)
		for(pos.y = ysize - 1; pos.y >= 0; --pos.y)
		for(pos.z = 0; pos.z < 1; ++pos.z)
		{
			const auto type = static_cast<block::enums::type>(nazi[pos.y][pos.x]);
			g.world.set_block(pos + start_pos, g.block_registry.get_default(type));
		}
	});

	COMMAND("open_gui")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: open_gui <string: type>\n";
			return;
		}
		const string& type = args[0];
		if(g.gui->type() == type)
		{
			return;
		}

		unique_ptr<graphics::gui::Base> gui;
		try
		{
			gui = g.make_gui(type);
		}
		catch(const std::exception& e)
		{
			LOG(ERROR) << "got " << util::demangled_name(e) << " while opening \"" << type << "\" GUI: " << e.what() << '\n';
			return;
		}

		if(gui == nullptr)
		{
			LOG(ERROR) << "no such GUI type: " << type << '\n';
			return;
		}
		g.open_gui(std::move(gui));
	});
	COMMAND("close_gui")
	{
		g.gui->close();
	});

	#undef COMMAND
}

}
