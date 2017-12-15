#include "Game.hpp"

#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "ResourceManager.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/Enum/Type.hpp"
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
#include "graphics/Image.hpp"
#include "graphics/RenderWorld.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/Console.hpp"
#include "graphics/GUI/Pause.hpp"
#include "graphics/GUI/Play.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/ray.hpp"
#include "physics/RaycastHit.hpp"
#include "plugin/PluginManager.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/filesystem.hpp"
#include "util/key_press.hpp"
#include "util/logger.hpp"

using std::nullopt;
using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace block_thingy {

Game* Game::instance = nullptr;

struct Game::impl
{
	impl(Game& game)
	:
		game(game),
		delta_time(0),
		fps(999),
		just_opened_gui(false),
		last_key(0),
		last_key_scancode(0),
		consume_key_release(0),
		consume_key_release_scancode(0)
	{
	}

	Game& game;

	double delta_time;
	FPSManager fps;
	std::tuple<uint64_t, uint64_t> draw_stats;

	void find_hovered_block();

	std::vector<Command> commands;
	void add_commands();

	unique_ptr<graphics::gui::Base> temp_gui;

	bool just_opened_gui;
	int last_key;
	int last_key_scancode;
	int consume_key_release;
	int consume_key_release_scancode;
};

static unique_ptr<mesher::Base> make_mesher(const string& name)
{
	unique_ptr<mesher::Base> mesher;
	if(name == "Greedy")
	{
		return std::make_unique<mesher::Greedy>();
	}
	else if(name == "Simple")
	{
		return std::make_unique<mesher::Simple>();
	}
	else if(name == "Simple2")
	{
		return std::make_unique<mesher::Simple2>();
	}
	LOG(ERROR) << "No such mesher: " << name << '\n';
	return make_mesher("Simple2");
}

Game::Game()
:
	set_instance(this),
	camera(gfx),
	world("worlds/test", block_registry, make_mesher(settings::get<string>("mesher"))),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	keybinder(*Console::instance),
	render_distance(1),
	pImpl(std::make_unique<impl>(*this))
{
	resource_manager.load_blocks(*this);

	if(block_registry.get_extid_map().empty())
	{
		// this should be done when starting a new world, but the design does not work that way yet
		block_registry.reset_extid_map();
	}

	gui = std::make_unique<graphics::gui::Play>(*this);
	gui->init();

	gfx.hook_events(event_manager);

	pImpl->add_commands();
	Console::instance->run_line("exec binds");

	update_framebuffer_size(gfx.window_size);

	glfwSetJoystickCallback([](int joystick, int event)
	{
		if(joystick == GLFW_JOYSTICK_1 && event == GLFW_DISCONNECTED)
		{
			Game::instance->player.set_analog_motion({ 0, 0 });
		}
	});
	glfwSetWindowFocusCallback(gfx.window, []([[maybe_unused]] GLFWwindow* window, int focused)
	{
		assert(window == Gfx::instance->window);
		if(!focused)
		{
			if(Game::instance->gui->type() == "Play")
			{
				Console::instance->run_line("open_gui Pause");
			}
		}
		// check if pause because a focus event is sent when the game starts
		else if(Game::instance->gui->type() == "Pause")
		{
			// when the game is paused after losing focus, the cursor stays hidden
			// GLFW ignores setting the cursor to its current state, so re-hide it first
			glfwSetInputMode(Game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetInputMode(Game::instance->gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

Game::~Game()
{
	settings::save();
}

void Game::draw()
{
	// TODO: use double when available
	const float global_time = static_cast<float>(world.get_time());
	resource_manager.foreach_ShaderProgram([global_time](Resource<graphics::opengl::ShaderProgram> r)
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
			if(std::abs(axis) < 0.1f)
			{
				return 0;
			}
			return axis;
		};

		const float* axises = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		player.set_analog_motion({ fix_axis(axises[0]), fix_axis(axises[1]) });
		glm::dvec2 motion(fix_axis(axises[3]), fix_axis(axises[4]));
		joymove(motion);
	}

	pImpl->delta_time = pImpl->fps.enforceFPS();
}

void Game::step_world()
{
	player.rotation = camera.rotation;
	world.step(pImpl->delta_time);
	pImpl->find_hovered_block();
}

void Game::draw_world()
{
	draw_world(camera.position, camera.rotation, gfx.projection_matrix);
}

void Game::draw_world
(
	const glm::dvec3& cam_position,
	const glm::dvec3& cam_rotation,
	const glm::dmat4& projection_matrix
)
{
	draw_world(cam_position, Gfx::make_rotation_matrix(cam_rotation), projection_matrix);
}

void Game::draw_world
(
	const glm::dvec3& cam_position,
	const glm::dmat4& cam_rotation,
	const glm::dmat4& projection_matrix
)
{
	const bool wireframe = settings::get<bool>("wireframe");
	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	gfx.set_camera_view(cam_position, cam_rotation, projection_matrix);
	position::BlockInWorld render_origin(cam_position);
	const std::tuple<uint64_t, uint64_t> draw_stats = graphics::draw_world
	(
		world,
		resource_manager,
		gfx.vp_matrix,
		render_origin,
		render_distance
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

	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Game::open_gui(unique_ptr<graphics::gui::Base> gui)
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
	this->gui->mousemove(x, y);

	pImpl->just_opened_gui = true;
}

void Game::close_gui()
{
	// code may be running in the GUI, such as from clicking the Resume button in the pause menu
	// after immediate destructing, graphics::gui::widget::Container will continue its mousepress loop
	// this could crash the engine, so temp_gui keeps it for the rest of the frame
	pImpl->temp_gui = std::move(gui);
	gui = std::move(pImpl->temp_gui->parent);
	gui->init();
}

void Game::quit()
{
	glfwSetWindowShouldClose(gfx.window, GL_TRUE);
}

void Game::screenshot(fs::path path) const
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
	graphics::Image(width, height, std::move(pixels)).write(path);
}

double Game::get_fps() const
{
	return pImpl->fps.getFPS();
}

position::ChunkInWorld::value_type Game::get_render_distance() const
{
	return render_distance;
}

std::tuple<uint64_t, uint64_t> Game::get_draw_stats() const
{
	return pImpl->draw_stats;
}

void Game::update_framebuffer_size(const window_size_t& window_size)
{
	gfx.update_framebuffer_size(window_size);
	event_manager.do_event(Event_window_size_change(window_size));
}

void Game::keypress(const util::key_press& press)
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

void Game::charpress(const util::char_press& press)
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

void Game::mousepress(const util::mouse_press& press)
{
	gui->mousepress(press);
}

void Game::mousemove(const double x, const double y)
{
	gui->mousemove(x, y);
}

void Game::joypress(const int joystick, const int button, const bool pressed)
{
	gui->joypress(joystick, button, pressed);
}

void Game::joymove(const glm::dvec2& motion)
{
	gui->joymove(motion);
}

void Game::impl::find_hovered_block()
{
	physics::ray ray = physics::screen_pos_to_world_ray
	(
		game.gfx.window_mid,
		game.gfx.window_size,
		game.gfx.view_matrix_graphical,
		game.gfx.projection_matrix
	);

	glm::dvec3 offset = game.gfx.physical_position - game.gfx.graphical_position;
	ray.origin += offset;
	game.hovered_block = physics::raycast
	(
		game.world,
		ray,
		game.player.reach_distance
	);
}

void Game::impl::add_commands()
{
	// [[maybe_unused]] does not work with lambda captures
	// preprocessor directives do not work in #define, so I can not use #pragma clang diagnostic ...
	#define COMMAND(name) commands.emplace_back(*Console::instance, name, \
	[ \
		&game=game, \
		&player=game.player \
	] \
	( \
		[[maybe_unused]] const std::vector<string>& args \
	)

	COMMAND("save")
	{
		game.world.save();
	});
	COMMAND("quit")
	{
		game.quit();
	});

	COMMAND("break_block")
	{
		if(game.hovered_block == nullopt)
		{
			return;
		}

		const position::BlockInWorld pos = game.hovered_block->pos;
		shared_ptr<block::Base> block = game.world.get_block(pos);
		if(block->type() != block::enums::Type::none) // TODO: breakability check
		{
			game.world.set_block(pos, game.block_registry.get_default(block::enums::Type::air), false);
		}
	});
	COMMAND("place_block")
	{
		if(game.hovered_block == nullopt || game.copied_block == nullptr)
		{
			return;
		}

		shared_ptr<block::Base> block = game.copied_block;
		const position::BlockInWorld pos = game.hovered_block->adjacent();
		if(game.world.get_block(pos)->is_replaceable_by(*block)
		&& (player.can_place_block_at(pos) || !block->is_solid()))
		{
			game.world.set_block(pos, block, false);
		}
	});
	COMMAND("copy_block")
	{
		if(game.hovered_block != nullopt)
		{
			game.copied_block = game.world.get_block(game.hovered_block->pos);
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
			game.copied_block = game.block_registry.get_default(args[0]);
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
		if(game.hovered_block != nullopt)
		{
			game.world.get_block(game.hovered_block->pos)->use_start
			(
				game,
				game.world,
				player,
				game.hovered_block->pos,
				game.hovered_block->face()
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
			game.camera.rotation.x += value;
		}
		else if(part == "y")
		{
			game.camera.rotation.y += value;
		}
		else if(part == "z")
		{
			game.camera.rotation.z += value;
		}
		else
		{
			LOG(ERROR) << "component name must be x, y, or z\n";
			return;
		}
		LOG(INFO) << "camera rotation: " << glm::to_string(game.camera.rotation) << '\n';
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
			game.screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error saving screenshot: " << e.what() << '\n';
		}
	});

	COMMAND("render_distance++")
	{
		if(std::numeric_limits<decltype(game.render_distance)>::max() > game.render_distance)
		{
			game.render_distance += 1;
		}
		LOG(INFO) << "render distance: " << game.render_distance << '\n';
	});
	COMMAND("render_distance--")
	{
		game.render_distance -= 1;
		if(game.render_distance < 0)
		{
			game.render_distance = 0;
		}
		LOG(INFO) << "render distance: " << game.render_distance << '\n';
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
		if(game.hovered_block == nullopt || game.copied_block == nullptr)
		{
			return;
		}

		const position::BlockInWorld start_pos = game.hovered_block->adjacent();
		const position::BlockInWorld::value_type ysize = 9;
		const position::BlockInWorld::value_type xsize = 9;
		const block::enums::Type_t i = static_cast<block::enums::Type_t>(game.copied_block->type()); // TODO: use copied_block instance
		block::enums::Type_t nazi[ysize][xsize]
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
		position::BlockInWorld pos;
		for(pos.x = 0; pos.x < xsize; ++pos.x)
		{
			for(pos.y = ysize - 1; pos.y >= 0; --pos.y)
			{
				for(pos.z = 0; pos.z < 1; ++pos.z)
				{
					const auto type = static_cast<block::enums::Type>(nazi[pos.y][pos.x]);
					game.world.set_block(pos + start_pos, game.block_registry.get_default(type));
				}
			}
		}
	});

	COMMAND("open_gui")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: open_gui <string: name>\n";
			return;
		}
		const string name = args[0];
		unique_ptr<graphics::gui::Base> gui;
		if(game.gui->type() == name)
		{
			return;
		}
		if(name == "Pause")
		{
			gui = std::make_unique<graphics::gui::Pause>(game);
		}
		else if(name == "Console")
		{
			gui = std::make_unique<graphics::gui::Console>(game);
		}
		else
		{
			LOG(ERROR) << "No such GUI: " << name << '\n';
			return;
		}
		game.open_gui(std::move(gui));
	});
	COMMAND("close_gui")
	{
		game.gui->close();
	});

	#undef COMMAND
}

}
