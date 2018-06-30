#include "game.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
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
#include "block/block.hpp"
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
#include "graphics/GUI/main_menu.hpp"
#include "graphics/GUI/new_world.hpp"
#include "graphics/GUI/Pause.hpp"
#include "graphics/GUI/Play.hpp"
#include "graphics/GUI/singleplayer.hpp"
#include "graphics/opengl/push_state.hpp"
#include "input/joy_press.hpp"
#include "input/key_press.hpp"
#include "physics/ray.hpp"
#include "physics/raycast_hit.hpp"
#include "physics/raycast_util.hpp"
#include "plugin/PluginManager.hpp"
#include "position/block_in_world.hpp"
#include "util/demangled_name.hpp"
#include "util/filesystem.hpp"
#include "util/grisu2.hpp"
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
	explicit impl(game& g)
	:
		g(g),
		delta_time(0),
		fps(999),
		global_ticks(0),
		root_gui(nullptr),
		just_opened_gui(false),
		last_key(0),
		last_key_scancode(0),
		consume_key_release(0),
		consume_key_release_scancode(0)
	{
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	game& g;

	double delta_time;
	fps_manager fps;
	uint64_t global_ticks;
	std::tuple<uint64_t, uint64_t> draw_stats;

	void find_hovered_block();

	std::vector<Command> commands;
	void add_commands();

	std::unordered_map<int, uint64_t> joystate;
	void do_joystick_input(int joystick);

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
	if(name == "simple")
	{
		return std::make_unique<mesher::simple>();
	}
	if(name == "simple2")
	{
		return std::make_unique<mesher::simple2>();
	}
	LOG(ERROR) << "No such mesher: " << name << '\n';
	return make_mesher("simple2");
}

game::game()
:
	set_instance(this),
	keybinder(*Console::instance),
	pImpl(std::make_unique<impl>(*this))
{
	register_gui<graphics::gui::Console>("console");
	register_gui<graphics::gui::main_menu>("main_menu");
	register_gui<graphics::gui::new_world>("new_world");
	register_gui<graphics::gui::Pause>("pause");
	register_gui<graphics::gui::Play>("play");
	register_gui<graphics::gui::singleplayer>("singleplayer");

	gui = make_gui("main_menu");
	pImpl->root_gui = gui.get();
	gui->switch_to();

	gfx.hook_events(event_manager);

	pImpl->add_commands();
	Console::instance->run_line("exec binds");

	update_framebuffer_size(gfx.window_size);

	glfwSetJoystickCallback([](const int joystick, const int event)
	{
		if(joystick == GLFW_JOYSTICK_1 && event == GLFW_DISCONNECTED && game::instance->player != nullptr)
		{
			game::instance->player->set_analog_motion({ 0, 0 });
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
		// note: a focus event is sent when the engine starts, so this applies only when the root GUI does not hide the cursor
		else
		{
			// when the game is paused after losing focus, the cursor stays hidden
			// GLFW ignores setting the cursor to its current state, so re-hide it first
			glfwSetInputMode(game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetInputMode(game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	});

	event_manager.add_handler(EventType::change_setting, [&game=*this](const Event& event)
	{
		if(game.world == nullptr)
		{
			return;
		}

		const auto& e = static_cast<const Event_change_setting&>(event);

		if(e.name == "mesher")
		{
			const string name = *e.new_value.get<string>();
			game.world->set_mesher(make_mesher(name));
		}
	});

	PluginManager::instance->plugin_init(*this);
}

game::~game()
{
	settings::save();
}

void game::draw()
{
	// TODO: use double when available
	const float global_time = static_cast<float>(get_global_time());
	const float world_time = world != nullptr ? static_cast<float>(world->get_time()) : 0;
	resource_manager.foreach_shader_program([global_time, world_time](resource<graphics::opengl::shader_program> r)
	{
		r->uniform("global_time", global_time);
		r->uniform("world_time", world_time);
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
	gfx.screen_shader->use();
	gfx.quad_vao.draw(GL_TRIANGLES, 0, 6);

	if(settings::get<bool>("screen_shader_debug"))
	{
		glViewport(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
		glScissor(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
		glEnable(GL_SCISSOR_TEST);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
		gui->draw();
	}


	glfwSwapBuffers(gfx.window);

	glfwPollEvents();
	pImpl->just_opened_gui = false;

	for(int joystick = GLFW_JOYSTICK_1; joystick <= GLFW_JOYSTICK_LAST; ++joystick)
	{
		pImpl->do_joystick_input(joystick);
	}

	pImpl->delta_time = pImpl->fps.enforce_fps();

	// TODO: draw and step on different threads
	step();
}

void game::step()
{
	if(gui->type() == "play")
	{
		assert(player != nullptr);
		assert(world != nullptr);
		player->rotation = camera.rotation;
		world->step(pImpl->delta_time);
		pImpl->find_hovered_block();
	}
	++pImpl->global_ticks;
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
	assert(world != nullptr);

	const bool wireframe = settings::get<bool>("wireframe");
	graphics::opengl::push_state<GLenum, GL_POLYGON_MODE> _polygon_mode(wireframe ? GL_LINE : GL_FILL);

	gfx.set_camera_view(cam_position, cam_rotation, projection_matrix);
	position::block_in_world render_origin(cam_position);
	const std::tuple<uint64_t, uint64_t> draw_stats = graphics::draw_world
	(
		*world,
		resource_manager,
		camera,
		gfx.vp_matrix,
		render_origin,
		static_cast<uint64_t>(settings::get<int64_t>("render_distance"))
	);
	pImpl->draw_stats =
	{
		std::get<0>(pImpl->draw_stats) + std::get<0>(draw_stats),
		std::get<1>(pImpl->draw_stats) + std::get<1>(draw_stats),
	};

	assert(player != nullptr);
	if(player->hovered_block != nullopt
	&& settings::get<bool>("show_HUD"))
	{
		const glm::dvec4 color = world->block_manager.info.selection_color(player->hovered_block->block);
		gfx.draw_block_outline(player->hovered_block->pos, color);
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

void game::open_gui(unique_ptr<graphics::gui::Base> new_gui)
{
	if(new_gui == nullptr)
	{
		LOG(WARN) << "Tried to open a null GUI\n";
		return;
	}

	new_gui->parent = std::move(gui);
	gui = std::move(new_gui);
	gui->parent->switch_from();
	gui->switch_to();

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
	pImpl->temp_gui->switch_from();
	gui->switch_to();
}

void game::quit()
{
	if(world != nullptr)
	{
		world->save_all();
	}
	if(gui->parent == nullptr)
	{
		assert(gui.get() == pImpl->root_gui);
		glfwSetWindowShouldClose(gfx.window, true);
		return;
	}
	pImpl->temp_gui = std::move(gui);
	gui = std::move(pImpl->temp_gui->parent);
	while(gui->parent != nullptr)
	{
		gui = std::move(gui->parent);
	}
	assert(gui.get() == pImpl->root_gui);
	pImpl->temp_gui->switch_from();
	gui->switch_to();
	player = nullptr;
	world = nullptr;
}

void game::new_world(fs::path path, const string& name, const double seed)
{
	load_world(std::move(path));
	resource_manager.load_blocks(world->block_manager);
	world->set_name(name);
	world->set_seed(seed);
	world->save_all();
}

void game::load_world(fs::path path)
{
	if(path.is_relative() && (fs::is_directory("worlds") || fs::create_directory("worlds")))
	{
		path = "worlds" / path;
	}
	if(world != nullptr)
	{
		LOG(ERROR) << "can not load world " << path.u8string() << " because a world is already loaded\n";
		return;
	}
	LOG(INFO) << "loading world " << path.u8string() << '\n';
	world = std::make_shared<world::world>(path, make_mesher(settings::get<string>("mesher")));
	player = world->add_player("test_player");
	PluginManager::instance->plugin_load_world(*world);
	open_gui(make_gui("play"));
}

void game::screenshot(fs::path path) const
{
	if(path.is_relative() && (fs::is_directory("screenshots") || fs::create_directory("screenshots")))
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

uint64_t game::get_global_ticks() const
{
	return pImpl->global_ticks;
}

double game::get_global_time() const
{
	return pImpl->global_ticks / 60.0;
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

void game::keypress(const input::key_press& press)
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

void game::charpress(const input::char_press& press)
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

void game::mousepress(const input::mouse_press& press)
{
	gui->mousepress(press);
}

void game::mousemove(const glm::dvec2& position)
{
	gui->mousemove(position);
}

void game::joypress(const input::joy_press& press)
{
	static_assert(GLFW_JOYSTICK_1 == 0);
	const int key = 1000 * (press.joystick + 1) + press.button;
	keypress({key, 0, press.action, 0});
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
	g.player->hovered_block = physics::raycast
	(
		*g.world,
		ray,
		g.player->reach_distance
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

	#define ASSERT_IN_GAME(name) \
	if(g.world == nullptr) \
	{ \
		LOG(ERROR) << name " command can be used only in-game\n"; \
		return; \
	}

	COMMAND("save")
	{
		ASSERT_IN_GAME("save");
		g.world->save_all();
	});
	COMMAND("quit")
	{
		g.quit();
	});

	COMMAND("break_block")
	{
		ASSERT_IN_GAME("break_block");

		if(player->hovered_block == nullopt)
		{
			return;
		}

		const position::block_in_world pos = player->hovered_block->pos;
		const block_t old_block = player->hovered_block->block;
		const block_t new_block = g.world->block_manager.process_break
		(
			g,
			*player,
			*g.world,
			pos,
			player->hovered_block->face(),
			old_block
		);
		if(new_block != old_block)
		{
			g.world->set_block(pos, new_block, false);
		}
	});
	COMMAND("place_block")
	{
		ASSERT_IN_GAME("place_block");

		if(player->copied_block == nullopt
		|| player->hovered_block == nullopt)
		{
			return;
		}

		const position::block_in_world pos = player->hovered_block->adjacent();
		const block_t old_block = g.world->get_block(pos);
		const block_t new_block = g.world->block_manager.process_place
		(
			g,
			*player,
			*g.world,
			pos,
			player->hovered_block->face(),
			old_block
		);
		if(new_block != old_block)
		{
			g.world->set_block(pos, new_block, false);
		}
	});
	COMMAND("copy_block")
	{
		ASSERT_IN_GAME("copy_block");

		if(player->hovered_block != nullopt)
		{
			player->copied_block = player->hovered_block->block;
		}
	});
	COMMAND("set_block")
	{
		ASSERT_IN_GAME("set_block");

		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: set_block <string: strid>\n";
			return;
		}
		try
		{
			player->copied_block = g.world->block_manager.get_block(args[0]);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << e.what() << '\n';
		}
	});

	COMMAND("+forward")
	{
		ASSERT_IN_GAME("+forward");
		player->move_forward(true);
	});
	COMMAND("-forward")
	{
		ASSERT_IN_GAME("-forward");
		player->move_forward(false);
	});
	COMMAND("+backward")
	{
		ASSERT_IN_GAME("+backward");
		player->move_backward(true);
	});
	COMMAND("-backward")
	{
		ASSERT_IN_GAME("-backward");
		player->move_backward(false);
	});
	COMMAND("+left")
	{
		ASSERT_IN_GAME("+left");
		player->move_left(true);
	});
	COMMAND("-left")
	{
		ASSERT_IN_GAME("-left");
		player->move_left(false);
	});
	COMMAND("+right")
	{
		ASSERT_IN_GAME("+right");
		player->move_right(true);
	});
	COMMAND("-right")
	{
		ASSERT_IN_GAME("-right");
		player->move_right(false);
	});
	COMMAND("jump")
	{
		ASSERT_IN_GAME("jump");
		player->jump();
	});
	COMMAND("+crouch")
	{
		ASSERT_IN_GAME("+crouch");
		player->crouching = true;
	});
	COMMAND("-crouch")
	{
		ASSERT_IN_GAME("-crouch");
		player->crouching = false;
	});
	COMMAND("+use")
	{
		ASSERT_IN_GAME("+use");

		if(player->hovered_block == nullopt)
		{
			return;
		}

		const physics::raycast_hit& hovered = *player->hovered_block;
		player->block_being_used = hovered;
		g.world->block_manager.start_use
		(
			g,
			*player,
			*g.world,
			hovered.pos,
			hovered.face(),
			hovered.block
		);
		/* TODO:
		when the block is unselected during use:
			call -use on the block
			? call +use on the new selected block
		when the block is changed:
			? call -use before changing
			? call +use on the new block
		*/
	});
	COMMAND("-use")
	{
		ASSERT_IN_GAME("-use");

		if(player->block_being_used == nullopt)
		{
			return;
		}

		const auto& used = *player->block_being_used;
		g.world->block_manager.end_use
		(
			g,
			*player,
			*g.world,
			used.pos,
			used.face(),
			used.block
		);
	});
	COMMAND("+sprint")
	{
		ASSERT_IN_GAME("+sprint");
		player->go_faster(true);
	});
	COMMAND("-sprint")
	{
		ASSERT_IN_GAME("-sprint");
		player->go_faster(false);
	});

	COMMAND("noclip")
	{
		ASSERT_IN_GAME("noclip");
		player->toggle_noclip();
	});
	COMMAND("respawn")
	{
		ASSERT_IN_GAME("respawn");
		player->respawn();
		LOG(INFO) << "respawned at " << glm::to_string(player->position()) << '\n';
	});

	COMMAND("save_pos")
	{
		ASSERT_IN_GAME("save_pos");

		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: save_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		std::ofstream streem(save_name);
		streem << util::os_grisu2;

		const glm::dvec3 pos = player->position();
		streem << pos.x << ' ' << pos.y << ' ' << pos.z << '\n';

		const glm::dvec3 rot = player->rotation();
		streem << rot.x << ' ' << rot.y << ' ' << rot.z << '\n';

		streem.flush();
		LOG(INFO) << "saved position and rotation to " << save_name << '\n';
	});
	COMMAND("load_pos")
	{
		ASSERT_IN_GAME("load_pos");

		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: load_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		if(!fs::is_regular_file(save_name))
		{
			LOG(ERROR) << "file not found: " << save_name << '\n';
			return;
		}
		std::ifstream streem(save_name);

		glm::dvec3 pos;
		streem >> pos.x;
		streem >> pos.y;
		streem >> pos.z;
		player->position = pos;
		LOG(INFO) << "set position to " << glm::to_string(pos) << '\n';

		glm::dvec3 rot;
		streem >> rot.x;
		streem >> rot.y;
		streem >> rot.z;
		player->rotation = rot;
		LOG(INFO) << "set rotation to " << glm::to_string(rot) << '\n';
	});

	COMMAND("cam.rot")
	{
		ASSERT_IN_GAME("cam.rot");

		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: cam.rot x|y|z <number: degrees>\n";
			return;
		}
		const string part = args[0];
		const string svalue = args[1];
		const std::optional<double> value = util::stod(svalue);
		if(value == nullopt)
		{
			LOG(ERROR) << "not a number or out of range: " << svalue << '\n';
			return;
		}
		if(part == "x")
		{
			g.camera.rotation.x += *value;
		}
		else if(part == "y")
		{
			g.camera.rotation.y += *value;
		}
		else if(part == "z")
		{
			g.camera.rotation.z += *value;
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
		if(args.empty())
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
		ASSERT_IN_GAME("reach_distance++");

		player->reach_distance += 1;
		LOG(INFO) << "reach distance: " << player->reach_distance << '\n';
	});
	COMMAND("reach_distance--")
	{
		ASSERT_IN_GAME("reach_distance--");

		player->reach_distance -= 1;
		LOG(INFO) << "reach distance: " << player->reach_distance << '\n';
	});

	COMMAND("nazi")
	{
		ASSERT_IN_GAME("nazi");

		if(player->copied_block == nullopt
		|| player->hovered_block == nullopt)
		{
			return;
		}

		const position::block_in_world start_pos = player->hovered_block->adjacent();
		const position::block_in_world::value_type ysize = 9;
		const position::block_in_world::value_type xsize = 9;
		const block_t i = *player->copied_block;
		const block_t n;
		const block_t nazi[ysize][xsize]
		{
			{ i, n, n, n, i, i, i, i, i, },
			{ i, n, n, n, i, n, n, n, n, },
			{ i, n, n, n, i, n, n, n, n, },
			{ i, n, n, n, i, n, n, n, n, },
			{ i, i, i, i, i, i, i, i, i, },
			{ n, n, n, n, i, n, n, n, i, },
			{ n, n, n, n, i, n, n, n, i, },
			{ n, n, n, n, i, n, n, n, i, },
			{ i, i, i, i, i, n, n, n, i, },
		};
		position::block_in_world pos;
		for(pos.x = 0; pos.x < xsize; ++pos.x)
		for(pos.y = ysize - 1; pos.y >= 0; --pos.y)
		for(pos.z = 0; pos.z < 1; ++pos.z)
		{
			const block_t block = nazi[pos.y][pos.x];
			if(block != n)
			{
				g.world->set_block(pos + start_pos, block);
			}
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
		assert(g.gui != nullptr);
		g.gui->close();
	});

	COMMAND("set_world_name")
	{
		ASSERT_IN_GAME("set_world_name");
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: set_world_name <string: name>\n";
			return;
		}
		g.world->set_name(args[0]);
	});

	#undef ASSERT_IN_GAME
	#undef COMMAND
}

void game::impl::do_joystick_input(const int joystick)
{
	int button_count;
	const unsigned char* buttons = glfwGetJoystickButtons(joystick, &button_count);
	if(buttons == nullptr) // joystick not present
	{
		return;
	}
	assert(button_count < 256);
	for(int button = 0; button < button_count; ++button)
	{
		auto& button_time = joystate[joystick * 256 + button];

		if(buttons[button] == GLFW_RELEASE)
		{
			if(button_time != 0)
			{
				button_time = 0;
				g.joypress({joystick, button, GLFW_RELEASE});
			}
			continue;
		}

		++button_time;
		if(button_time == 1)
		{
			g.joypress({joystick, button, GLFW_PRESS});
			continue;
		}

		if(button_time >= 30 && button_time % 15 == 0)
		{
			g.joypress({joystick, button, GLFW_REPEAT});
			continue;
		}
	}

	int axis_count;
	const float* axes = glfwGetJoystickAxes(joystick, &axis_count);
	if(axes == nullptr) // joystick not present
	{
		return;
	}
	assert(axis_count % 2 == 0);

	assert(axis_count == 8); // assume XInput
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

	const double deadzone = settings::get<double>("joystick_deadzone");
	auto fix_axes = [deadzone](const float x_, const float y_) -> glm::dvec2
	{
		const double x = static_cast<double>(x_);
		const double y = static_cast<double>(y_);
		if(std::abs(x) < deadzone && std::abs(y) < deadzone)
		{
			return {0, 0};
		}
		return {x, y};
	};

	glm::dvec2 stickL = (fix_axes(axes[0], axes[1]));
	glm::dvec2 stickR = (fix_axes(axes[3], axes[4]));

	if(g.player != nullptr)
	{
		g.player->set_analog_motion(stickL);
	}
	if(stickR.x != 0 || stickR.y != 0)
	{
		g.joymove(stickR);
	}
}

}
