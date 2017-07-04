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

#include <easylogging++/easylogging++.hpp>
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
#include "block/Air.hpp"
#include "block/Base.hpp"
#include "block/Glass.hpp"
#include "block/Light.hpp"
#include "block/None.hpp"
#include "block/Teleporter.hpp"
#include "block/Test.hpp"
#include "block/Unknown.hpp"
#include "block/Enum/Type.hpp"
#include "chunk/Mesher/Greedy.hpp"
#include "chunk/Mesher/Simple.hpp"
#include "chunk/Mesher/SimpleAO.hpp"
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
#include "physics/RaycastHit.hpp"
#include "plugin/PluginManager.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "shim/make_unique.hpp"
#include "util/filesystem.hpp"

using std::string;
using std::unique_ptr;

Game* Game::instance = nullptr;

struct Game::impl
{
	impl(Game& game)
	:
		game(game),
		delta_time(0),
		fps(999)
	{
	}

	Game& game;

	double delta_time;
	FPSManager fps;

	void find_hovered_block();

	std::vector<Command> commands;
	void add_commands();

	std::unique_ptr<Graphics::GUI::Base> temp_gui;
};

static unique_ptr<Mesher::Base> make_mesher(const string& name)
{
	unique_ptr<Mesher::Base> mesher;
	if(name == "Greedy")
	{
		return std::make_unique<Mesher::Greedy>();
	}
	else if(name == "Simple")
	{
		return std::make_unique<Mesher::Simple>();
	}
	else if(name == "SimpleAO")
	{
		return std::make_unique<Mesher::SimpleAO>();
	}
	LOG(ERROR) << "No such mesher: " << name;
	return make_mesher("SimpleAO");
}

Game::Game(GLFWwindow* window)
:
	set_instance(this),
	hovered_block(nullptr),
	gfx(window),
	camera(gfx, event_manager),
	world("worlds/test", block_registry, make_mesher(Settings::get<string>("mesher"))),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	keybinder(*Console::instance),
	render_distance(1),
	pImpl(std::make_unique<impl>(*this))
{
	// these must be added first (in this order!) to get the correct IDs
	add_block<Block::None>("none");
	add_block<Block::Air>("air");
	add_block<Block::Unknown>("unknown");

	add_block<Block::Test>("test");
	add_block<Block::Teleporter>("teleporter");
	add_block<Block::Light>("light");
	add_block<Block::Glass>("glass");

	resource_manager.load_blocks(*this);

	if(block_registry.get_extid_map().size() == 0)
	{
		// this should be done when starting a new world, but the design does not work that way yet
		block_registry.reset_extid_map();
	}

	block_type = block_registry.get_id("White");

	gui = std::make_unique<Graphics::GUI::Play>(*this);
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
	glfwSetWindowFocusCallback(gfx.window, [](GLFWwindow* window, int focused)
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
		auto e = static_cast<const Event_change_setting&>(event);

		if(e.name == "mesher")
		{
			const string name = *static_cast<const string*>(e.value);
			game.world.set_mesher(make_mesher(name));
		}
	});

	PluginManager::instance->init_plugins();
}

Game::~Game()
{
}

void Game::draw()
{
	// TODO: use double when available
	const float global_time = static_cast<float>(world.get_time());
	for(auto& p : gfx.block_shaders)
	{
		p.second.uniform("global_time", global_time);
	}

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
	glUseProgram(gfx.screen_shader->get_name());
	gfx.screen_shader->uniform("time", static_cast<float>(world.get_time()));
	gfx.quad_vao.draw(GL_TRIANGLES, 0, 6);

	// TODO: it might be faster to listen for the change event and set a private bool instead of getting the value every frame
	if(Settings::get<string>("screen_shader") != "default")
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

	if(glfwJoystickPresent(GLFW_JOYSTICK_1))
	{
		int count;

		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		for(int i = 0; i < count; ++i)
		{
			joypress(1, i, buttons[i] != 0);
		}

		auto fix_axis = [](float axis)
		{
			if(std::abs(axis) < 0.1f)
			{
				return 0.0f;
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
	glm::dmat4 view_matrix = Gfx::make_view_matrix(camera.rotation);
	draw_world(camera.position, view_matrix, gfx.projection_matrix);
}

void Game::draw_world
(
	const glm::dvec3& position,
	const glm::dmat4& view_matrix,
	const glm::dmat4& projection_matrix
)
{
	const bool wireframe = Settings::get<bool>("wireframe");
	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	gfx.set_camera_view(position, view_matrix, projection_matrix);
	Position::BlockInWorld render_origin(position);
	RenderWorld::draw_world
	(
		world,
		gfx.block_shaders,
		gfx.matriks,
		render_origin,
		render_distance
	);

	if(hovered_block != nullptr)
	{
		const glm::dvec4 color = world.get_block(hovered_block->pos).selection_color();
		gfx.draw_cube_outline(hovered_block->pos, color);
	}

	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Game::open_gui(unique_ptr<Graphics::GUI::Base> gui)
{
	if(gui == nullptr)
	{
		LOG(WARNING) << "Tried to open a null GUI";
		return;
	}
	gui->init();
	gui->parent = std::move(this->gui);
	this->gui = std::move(gui);

	double x, y;
	glfwGetCursorPos(gfx.window, &x, &y);
	this->gui->mousemove(x, y);
}

void Game::close_gui()
{
	// code may be running in the GUI, such as from clicking the Resume button in the pause menu
	// after immediate destructing, Graphics::GUI::Widget::Container will continue its mousepress loop
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
	LOG(INFO) << "saving screenshot to " << path.u8string();
	const auto width = gfx.window_size.x;
	const auto height = gfx.window_size.y;
	std::vector<uint8_t> pixels(4 * width * height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	Graphics::Image(width, height, std::move(pixels)).write(path);
}

double Game::get_fps() const
{
	return pImpl->fps.getFPS();
}

void Game::update_framebuffer_size(const window_size_t& window_size)
{
	gfx.update_framebuffer_size(window_size);
	event_manager.do_event(Event_window_size_change(window_size));
}

void Game::keypress(const Util::key_press& press)
{
	gui->keypress(press);
}

void Game::charpress(const Util::char_press& press)
{
	gui->charpress(press);
}

void Game::mousepress(const Util::mouse_press& press)
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

static void add_shader(Game& game, const Block::Enum::Type t, const fs::path& shader_path)
{
	bool is_new;
	try
	{
		is_new = game.gfx.block_shaders.emplace(t, "shaders/block" / shader_path).second;
	}
	catch(const std::runtime_error& e)
	{
		LOG(ERROR) << "shader error:\n" << e.what();
		is_new = game.gfx.block_shaders.emplace(t, "shaders/block/default").second;
	}
	if(is_new)
	{
		game.gfx.block_shaders[t].uniform("min_light", static_cast<float>(Settings::get<double>("min_light")));
	}
}

void Game::add_block(const string& strid, const Block::Enum::Type t, const fs::path& shader_path)
{
	LOG(DEBUG) << "ID " << t << ": " << strid;
	if(t == Block::Enum::Type::none || t == Block::Enum::Type::air)
	{
		return;
	}
	add_shader(*this, t, shader_path);
}

void Game::impl::find_hovered_block()
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay
	(
		game.gfx.window_mid,
		game.gfx.window_size,
		game.gfx.view_matrix_graphical,
		game.gfx.projection_matrix,
		out_origin,
		out_direction
	);

	glm::dvec3 offset = game.gfx.physical_position - game.gfx.graphical_position;
	game.hovered_block = PhysicsUtil::raycast
	(
		game.world,
		out_origin + offset,
		out_direction,
		game.player.reach_distance
	);
}

void Game::impl::add_commands()
{
	#define COMMAND_(name) commands.emplace_back(*Console::instance, name, [&game=game](
	#define COMMAND(name) COMMAND_(name))
	#define COMMAND_ARGS(name) COMMAND_(name)const std::vector<string>& args)

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
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->pos;
		if(game.world.get_block(pos).type() != Block::Enum::Type::none) // TODO: breakability check
		{
			game.world.set_block(pos, game.block_registry.make(Block::Enum::Type::air), false);
			//event_manager.do_event(Event_break_block(pos, face));
		}
	});
	COMMAND("place_block")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->adjacent();
		if(game.world.get_block(pos).is_replaceable())
		{
			// TODO: check solidity without constructing
			unique_ptr<Block::Base> block;
			if(game.copied_block != nullptr)
			{
				block = game.block_registry.make(*game.copied_block);
			}
			else
			{
				block = game.block_registry.make(game.block_type);
			}
			if(game.player.can_place_block_at(pos) || !block->is_solid())
			{
				game.world.set_block(pos, std::move(block), false);
				//event_manager.do_event(Event_place_block(pos, face));
			}
		}
	});
	COMMAND("copy_block")
	{
		if(game.hovered_block == nullptr)
		{
			game.copied_block = nullptr;
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->pos;
		const Block::Base& block = game.world.get_block(pos);
		game.block_type = block.type();
		game.copied_block = game.block_registry.make(block);
	});

	// TODO: less copy/paste
	COMMAND("+forward")
	{
		game.player.move_forward(true);
	});
	COMMAND("-forward")
	{
		game.player.move_forward(false);
	});
	COMMAND("+backward")
	{
		game.player.move_backward(true);
	});
	COMMAND("-backward")
	{
		game.player.move_backward(false);
	});
	COMMAND("+left")
	{
		game.player.move_left(true);
	});
	COMMAND("-left")
	{
		game.player.move_left(false);
	});
	COMMAND("+right")
	{
		game.player.move_right(true);
	});
	COMMAND("-right")
	{
		game.player.move_right(false);
	});
	COMMAND("jump")
	{
		game.player.jump();
	});
	COMMAND("+use")
	{
		if(game.hovered_block != nullptr)
		{
			game.world.get_block(game.hovered_block->pos).use_start();
		}
	});
	COMMAND("-use")
	{
		// TODO
	});
	COMMAND("+sprint")
	{
		game.player.go_faster(true);
	});
	COMMAND("-sprint")
	{
		game.player.go_faster(false);
	});

	COMMAND("noclip")
	{
		game.player.toggle_noclip();
	});
	COMMAND("respawn")
	{
		game.player.respawn();
		LOG(INFO) << "respawned at " << glm::to_string(game.player.position());
	});

	COMMAND_ARGS("save_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: save_pos <string: filename>";
			return;
		}
		const string save_name = args[0];
		std::ofstream streem(save_name);
		streem.precision(std::numeric_limits<double>::max_digits10);

		const glm::dvec3 pos = game.player.position();
		streem << pos.x << " " << pos.y << " " << pos.z << "\n";

		const glm::dvec3 rot = game.player.rotation();
		streem << rot.x << " " << rot.y << " " << rot.z;

		streem.flush();
		LOG(INFO) << "saved position and rotation to " << save_name;
	});
	COMMAND_ARGS("load_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: load_pos <string: filename>";
			return;
		}
		const string save_name = args[0];
		std::ifstream streem(save_name);

		glm::dvec3 pos;
		streem >> pos.x;
		streem >> pos.y;
		streem >> pos.z;
		game.player.position = pos;
		LOG(INFO) << "set position to " << glm::to_string(pos);

		glm::dvec3 rot;
		streem >> rot.x;
		streem >> rot.y;
		streem >> rot.z;
		game.player.rotation = rot;
		LOG(INFO) << "set rotation to " << glm::to_string(rot);
	});

	COMMAND_ARGS("cam.rot")
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: cam.rot x|y|z <float: degrees>";
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
			LOG(ERROR) << "component name must be x, y, or z";
			return;
		}
		LOG(INFO) << "camera rotation: " << glm::to_string(game.camera.rotation);
	});

	COMMAND_ARGS("screenshot")
	{
		string filename;
		if(args.size() == 0)
		{
			filename = Util::datetime() + ".png";
		}
		else if(args.size() == 1)
		{
			filename = args[0];
		}
		else
		{
			LOG(ERROR) << "Usage: screenshot [string: filename]";
			return;
		}
		try
		{
			game.screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error saving screenshot: " << e.what();
		}
	});

	COMMAND("render_distance++")
	{
		if(std::numeric_limits<decltype(game.render_distance)>::max() > game.render_distance)
		{
			game.render_distance += 1;
		}
		LOG(INFO) << "render distance: " << game.render_distance;
	});
	COMMAND("render_distance--")
	{
		game.render_distance -= 1;
		if(game.render_distance < 0)
		{
			game.render_distance = 0;
		}
		LOG(INFO) << "render distance: " << game.render_distance;
	});
	COMMAND("reach_distance++")
	{
		game.player.reach_distance += 1;
		LOG(INFO) << "reach distance: " << game.player.reach_distance;
	});
	COMMAND("reach_distance--")
	{
		game.player.reach_distance -= 1;
		LOG(INFO) << "reach distance: " << game.player.reach_distance;
	});

	COMMAND("block_type++")
	{
		auto i = static_cast<Block::Enum::Type_t>(game.block_type);
		i = (i + 1) % game.block_registry.get_max_id();
		if(i < 3)
		{
			i = 3;
		}
		game.block_type = static_cast<Block::Enum::Type>(i);
		game.copied_block = nullptr;
	});
	COMMAND("block_type--")
	{
		auto i = static_cast<Block::Enum::Type_t>(game.block_type);
		if(i == 3)
		{
			i = game.block_registry.get_max_id() - 1;
		}
		else
		{
			i = (i - 1) % game.block_registry.get_max_id();
		}
		game.block_type = static_cast<Block::Enum::Type>(i);
		game.copied_block = nullptr;
	});

	COMMAND("nazi")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld start_pos = game.hovered_block->adjacent();
		const Position::BlockInWorld::value_type ysize = 9;
		const Position::BlockInWorld::value_type xsize = 9;
		Block::Enum::Type_t nazi[ysize][xsize]
		{
			{ 2, 1, 1, 1, 2, 2, 2, 2, 2, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 2, 2, 2, 2, 2, 2, 2, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 2, 2, 2, 2, 2, 1, 1, 1, 2, },
		};
		Position::BlockInWorld pos;
		for(pos.x = 0; pos.x < xsize; ++pos.x)
		{
			for(pos.y = ysize - 1; pos.y >= 0; --pos.y)
			{
				for(pos.z = 0; pos.z < 1; ++pos.z)
				{
					const auto type = static_cast<Block::Enum::Type>(nazi[pos.y][pos.x]);
					game.world.set_block(pos + start_pos, game.block_registry.make(type));
				}
			}
		}
	});

	COMMAND_ARGS("open_gui")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: open_gui <GUI name>";
			return;
		}
		const string name = args[0];
		unique_ptr<Graphics::GUI::Base> gui;
		if(game.gui->type() == name)
		{
			return;
		}
		if(name == "Pause")
		{
			gui = std::make_unique<Graphics::GUI::Pause>(game);
		}
		else if(name == "Console")
		{
			gui = std::make_unique<Graphics::GUI::Console>(game);
		}
		else
		{
			LOG(ERROR) << "No such GUI: " << name;
			return;
		}
		game.open_gui(std::move(gui));
	});
	COMMAND("close_gui")
	{
		game.gui->close();
	});

	#undef COMMAND_ARGS
	#undef COMMAND
	#undef COMMAND_
}
