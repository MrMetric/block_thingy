#include "Game.hpp"

#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Poco/DateTimeFormatter.h>
#include <Poco/Timestamp.h>
#include <Poco/Timezone.h>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/RenderWorld.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/Pause.hpp"
#include "graphics/GUI/Play.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/RaycastHit.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::unique_ptr;

Game* Game::instance = nullptr;

Game::Game(Gfx& gfx)
	:
	block_type(BlockType::test),
	hovered_block(nullptr),
	gfx(gfx),
	camera(gfx, event_manager),
	world("worlds/test"),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	console(*this),
	keybinder(console),
	wireframe(false, [](const bool wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}),
	delta_time(0),
	fps(999),
	render_distance(3)
{
	Game::instance = this;

	gui = std::make_unique<Graphics::GUI::Play>(*this);
	gui->init();

	gfx.hook_events(event_manager);

	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(gfx.window_size);

	camera.rotation = player.rotation; // keep saved value
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gui->draw();

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
		player.set_analog_motion({ fix_axis(axises[0]), fix_axis(axises[1]) }); // TODO: set to 0 on unplug event
		glm::dvec2 motion(fix_axis(axises[3]), fix_axis(axises[4]));
		joymove(motion);
	}

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	ss << " | player.pos(" << glm::to_string(player.position) << ")";
	Position::BlockInWorld player_block_pos(player.position);
	ss << " | block" << player_block_pos;
	ss << " | chunk" << Position::ChunkInWorld(player_block_pos);
	ss << " | chunkblock" << Position::BlockInChunk(player_block_pos);
	glfwSetWindowTitle(gfx.window, ss.str().c_str());

	delta_time = fps.enforceFPS();
}

void Game::step_world()
{
	player.rotation = camera.rotation;
	world.step(delta_time);
	camera.position = player.position;
	camera.position.y += player.get_eye_height();
}

void Game::draw_world()
{
	gfx.set_camera_view(camera.position, camera.rotation);
	Position::BlockInWorld render_origin(player.position);
	RenderWorld::draw_world(world, gfx.block_shaders, gfx.matriks, render_origin, render_distance);
	find_hovered_block(gfx.projection_matrix, gfx.view_matrix_physical);
}

void Game::open_gui(std::unique_ptr<Graphics::GUI::Base> gui)
{
	if(gui == nullptr)
	{
		console.error_logger << "Tried to open a null GUI\n";
		return;
	}
	gui->init();
	gui->parent = std::move(this->gui);
	this->gui = std::move(gui);
}

void Game::quit()
{
	glfwSetWindowShouldClose(gfx.window, GL_TRUE);
}

#ifdef USE_LIBPNG
void Game::screenshot(const string& filename)
{
	console.logger << "saving screenshot to " << filename << "\n";
	const auto width = gfx.window_size.x;
	const auto height = gfx.window_size.y;
	auto pixels = std::make_unique<GLubyte[]>(3 * width * height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), width, height, true);
}
#endif

void Game::update_framebuffer_size(const window_size_t& window_size)
{
	event_manager.do_event(Event_window_size_change(window_size));
}

void Game::keypress(const int key, const int scancode, const int action, const int mods)
{
	gui->keypress(key, scancode, action, mods);
}

void Game::mousepress(const int button, const int action, const int mods)
{
	gui->mousepress(button, action, mods);
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

void Game::find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay(
		gfx.window_mid,
		gfx.window_size,
		view_matrix,
		projection_matrix,
		out_origin,
		out_direction
	);

	hovered_block = PhysicsUtil::raycast(world, out_origin, out_direction, player.reach_distance);
	if(hovered_block != nullptr)
	{
		const bool block_is_none = world.get_block(hovered_block->pos).type() == BlockType::none;
		glm::vec4 color = block_is_none ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 1, 1);
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	#define COMMAND_(name) commands.emplace_back(console, name, [](Game& game
	#define COMMAND(name) COMMAND_(name))
	#define COMMAND_ARGS(name) COMMAND_(name), const std::vector<string>& args)

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
		if(game.world.get_block(pos).type() != BlockType::none)
		{
			game.world.set_block(pos, game.block_registry.make(BlockType::air));
			game.find_hovered_block(game.gfx.projection_matrix, game.gfx.view_matrix_physical);
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
		if(game.world.get_block(pos).type() == BlockType::air && game.player.can_place_block_at(pos))
		{
			game.world.set_block(pos, game.block_registry.make(game.block_type));
			//event_manager.do_event(Event_place_block(pos, face));
		}
	});
	COMMAND("pick_block")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->pos;
		const Block::Base& block = game.world.get_block(pos);
		game.block_type = block.type();
		game.console.logger << "block type: " << block.type_id() << "\n";
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
	COMMAND("+sprint")
	{
		game.player.go_faster(true);
	});
	COMMAND("-sprint")
	{
		game.player.go_faster(false);
	});

	COMMAND("jump")
	{
		game.player.jump();
	});
	COMMAND("noclip")
	{
		game.player.toggle_noclip();
		game.console.logger << "noclip: " << (game.player.get_noclip() ? "true" : "false") << "\n";
	});
	COMMAND("respawn")
	{
		game.player.respawn();
		game.console.logger << "respawned at " << glm::to_string(game.player.position) << "\n";
	});

	COMMAND_ARGS("save_pos")
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: save_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		std::ofstream streem(save_name);
		streem.precision(std::numeric_limits<double>::max_digits10);
		streem << game.player.position.x << " " << game.player.position.y << " " << game.player.position.z << "\n";
		streem << game.player.rotation.x << " " << game.player.rotation.y << " " << game.player.rotation.z;
		streem.flush();
		game.console.logger << "saved position and rotation to " << save_name << "\n";
	});
	COMMAND_ARGS("load_pos")
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: load_pos <string: filename>\n";
			return;
		}
		const string save_name = args[0];
		std::ifstream streem(save_name);
		streem >> game.player.position.x;
		streem >> game.player.position.y;
		streem >> game.player.position.z;
		streem >> game.camera.rotation.x;
		streem >> game.camera.rotation.y;
		streem >> game.camera.rotation.z;
		game.console.logger << "set position to " << glm::to_string(game.player.position) << "\n";
		game.console.logger << "set rotation to " << glm::to_string(game.camera.rotation) << "\n";
	});

	COMMAND_ARGS("exec")
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: exec <string: filename>\n";
			return;
		}
		const string name = args[0];
		std::ifstream file("scripts/" + name);
		if(!file.is_open())
		{
			game.console.error_logger << "script not found: " << name << "\n";
			return;
		}
		for(string line; std::getline(file, line); )
		{
			game.console.run_line(line);
		}
	});

	COMMAND_ARGS("cam.rot")
	{
		if(args.size() != 2)
		{
			game.console.error_logger << "Usage: cam.rot x|y|z <float: degrees>\n";
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
			game.console.error_logger << "component name must be x, y, or z\n";
			return;
		}
		game.console.logger << "camera rotation: " << glm::to_string(game.camera.rotation) << "\n";
	});

	#ifdef USE_LIBPNG
	COMMAND_ARGS("screenshot")
	{
		string filename;
		if(args.size() == 0)
		{
			Poco::Timestamp t;
			const int tzd = Poco::Timezone::tzd();
			t += tzd * 1000000L;
			filename = Poco::DateTimeFormatter::format(t, "%Y-%m-%d %H:%M:%s %z.png", tzd);
		}
		else if(args.size() == 1)
		{
			filename = args[0];
		}
		else
		{
			game.console.error_logger << "Usage: screenshot [string: filename]\n";
			return;
		}
		try
		{
			game.screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			game.console.error_logger << "error saving screenshot: " << e.what() << "\n";
		}
	});
	#endif

	COMMAND("toggle_fullscreen")
	{
		game.gfx.toggle_fullscreen();
	});
	COMMAND("toggle_wireframe")
	{
		game.wireframe = !game.wireframe();
		game.console.logger << "wireframe: " << (game.wireframe() ? "true" : "false") << "\n";
	});
	COMMAND("toggle_cull_face")
	{
		game.gfx.toggle_cull_face();
		game.console.logger << "cull face: " << (game.gfx.cull_face ? "true" : "false") << "\n";
	});
	COMMAND_ARGS("fov")
	{
		if(args.size() != 1 || args[0].length() == 0)
		{
			game.console.error_logger << "Usage: fov <exact number or +- difference>\n";
			return;
		}

		const string svalue = args[0];
		const double value = std::stod(svalue);
		if(svalue[0] == '+' || svalue[0] == '-')
		{
			game.gfx.fov += value;
		}
		else
		{
			game.gfx.fov = value;
		}

		if(game.gfx.fov < 0)
		{
			game.gfx.fov = std::fmod(game.gfx.fov, 360) + 360;
		}
		else
		{
			game.gfx.fov = std::fmod(game.gfx.fov, 360);
		}
		if(game.gfx.fov == 0) // avoid division by zero
		{
			game.gfx.fov = 360;
		}

		game.gfx.update_projection_matrix();
	});

	COMMAND("render_distance++")
	{
		if(std::numeric_limits<decltype(game.render_distance)>::max() > game.render_distance)
		{
			game.render_distance += 1;
		}
		game.console.logger << "render distance: " << game.render_distance << "\n";
	});
	COMMAND("render_distance--")
	{
		game.render_distance -= 1;
		if(game.render_distance < 0)
		{
			game.render_distance = 0;
		}
		game.console.logger << "render distance: " << game.render_distance << "\n";
	});
	COMMAND("reach_distance++")
	{
		game.player.reach_distance += 1;
		game.console.logger << "reach distance: " << game.player.reach_distance << "\n";
	});
	COMMAND("reach_distance--")
	{
		game.player.reach_distance -= 1;
		game.console.logger << "reach distance: " << game.player.reach_distance << "\n";
	});

	COMMAND("block_type++")
	{
		block_type_id_t i = static_cast<block_type_id_t>(game.block_type);
		i = (i + 1) % BlockType_COUNT;
		if(i < 2)
		{
			i = 2;
		}
		game.block_type = static_cast<BlockType>(i);
		game.console.logger << "block type: " << i << "\n";
	});
	COMMAND("block_type--")
	{
		block_type_id_t i = static_cast<block_type_id_t>(game.block_type);
		if(i == 2)
		{
			i = BlockType_COUNT - 1;
		}
		else
		{
			i = (i - 1) % BlockType_COUNT;
		}
		game.block_type = static_cast<BlockType>(i);
		game.console.logger << "block type: " << i << "\n";
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
		block_type_id_t nazi[ysize][xsize]
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
					const BlockType type = static_cast<BlockType>(nazi[pos.y][pos.x]);
					game.world.set_block(pos + start_pos, game.block_registry.make(type));
				}
			}
		}
	});

	COMMAND_ARGS("open_gui")
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: open_gui <GUI name>\n";
			return;
		}
		const std::string name = args[0];
		std::unique_ptr<Graphics::GUI::Base> gui;
		if(name == "pause")
		{
			gui = std::make_unique<Graphics::GUI::Pause>(game);
		}
		else if(name == "console")
		{
			// TODO
		}
		else
		{
			game.console.error_logger << "No such GUI: " << name << "\n";
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
