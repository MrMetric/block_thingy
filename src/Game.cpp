#include "Game.hpp"

#include <cmath>
#include <ctime>
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

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "block/Block.hpp"
#include "block/BlockType.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/RenderWorld.hpp"
#include "gui/GUI.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/RaycastHit.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::unique_ptr;

Game* Game::instance = nullptr;

Game::Game(GLFWwindow* window, const window_size_t& window_size)
	:
	window(window),
	hovered_block(nullptr),
	camera(window, event_manager),
	gfx(window, event_manager),
	world("worlds/test"),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	gui(event_manager),
	wireframe(false, [](bool wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}),
	delta_time(0),
	fps(999),
	render_distance(3),
	keybinder(console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(window_size);

	camera.rotation = player.rotation; // keep saved value
}

void Game::draw()
{
	player.rotation = camera.rotation;
	world.step(delta_time);
	camera.position = player.position;
	camera.position.y += player.get_eye_height();

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	gfx.set_camera_view(camera.position, camera.rotation);
	Position::BlockInWorld render_origin(player.position);
	RenderWorld::draw_world(world, gfx.block_shaders, gfx.matriks, render_origin, render_distance);
	find_hovered_block(gfx.projection_matrix, gfx.view_matrix_physical);
	gui.draw(gfx);
	glfwSwapBuffers(window);

	glfwPollEvents();

	if(glfwJoystickPresent(GLFW_JOYSTICK_1))
	{
		int count;

		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		for(int i = 0; i < count; ++i)
		{
			keybinder.joypress(1, i, buttons[i] != 0);
		}

		auto fix_axis = [](float axis)
		{
			if(std::abs(axis) < 0.1)
			{
				return 0.0f;
			}
			return axis;
		};

		const float* axises = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		player.set_analog_motion({ fix_axis(axises[0]), fix_axis(axises[1]) });
		glm::dvec2 window_mid = glm::dvec2(gfx.window_size) / 2.0;
		glm::dvec2 motion(fix_axis(axises[3]), fix_axis(axises[4]));
		motion *= 32.0;
		motion += window_mid;
		camera.mousemove(motion.x, motion.y, true);
	}

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	ss << " | player.pos(" << glm::to_string(player.position) << ")";
	Position::BlockInWorld player_block_pos(player.position);
	ss << " | block" << player_block_pos;
	ss << " | chunk" << Position::ChunkInWorld(player_block_pos);
	ss << " | chunkblock" << Position::BlockInChunk(player_block_pos);
	glfwSetWindowTitle(window, ss.str().c_str());

	delta_time = fps.enforceFPS();
}

#ifdef USE_LIBPNG
void Game::screenshot(const string& filename)
{
	console.logger << "saving screenshot to " << filename << "\n";
	const auto width = gfx.window_size.x;
	const auto height = gfx.window_size.y;
	unique_ptr<GLubyte[]> pixels = std::make_unique<GLubyte[]>(3 * width * height);
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
	keybinder.keypress(key, scancode, action, mods);
}

static BlockType block_type = BlockType::test;
void Game::mousepress(const int button, const int action, const int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			console.run_command("break_block");
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			console.run_command("place_block");
		}
	}
}

void Game::mousemove(const double x, const double y)
{
	camera.mousemove(x, y);
}

void Game::find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay(
		glm::dvec2(gfx.window_size) / 2.0,
		gfx.window_size,
		view_matrix,
		projection_matrix,
		out_origin,
		out_direction
	);

	hovered_block = PhysicsUtil::raycast(world, out_origin, out_direction, player.reach_distance);
	if(hovered_block != nullptr)
	{
		bool block_is_none = world.get_block_const(hovered_block->pos).type() == BlockType::none;
		glm::vec4 color = block_is_none ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 1, 1);
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	#define COMMAND_(name) commands.emplace_back(console, name, [&game=*this]
	#define COMMAND(name) COMMAND_(name)()
	#define COMMAND_ARGS(name) COMMAND_(name)(const std::vector<string>& args)

	COMMAND("quit")
	{
		game.world.save();
		glfwSetWindowShouldClose(game.window, GL_TRUE);
	});

	COMMAND("break_block")
	{
		if(game.hovered_block != nullptr)
		{
			const Position::BlockInWorld pos = game.hovered_block->pos;
			if(game.world.get_block_const(pos).type() != BlockType::none)
			{
				game.world.set_block(pos, Block::Block(BlockType::air));
				game.find_hovered_block(game.gfx.projection_matrix, game.gfx.view_matrix_physical);
				//event_manager.do_event(Event_break_block(pos, face));
			}
		}
	});
	COMMAND("place_block")
	{
		if(game.hovered_block != nullptr)
		{
			const Position::BlockInWorld pos = game.hovered_block->adjacent();
			if(game.world.get_block_const(pos).type() == BlockType::air && game.player.can_place_block_at(pos))
			{
				game.world.set_block(pos, Block::Block(block_type));
				//event_manager.do_event(Event_place_block(pos, face));
			}
		}
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
		game.console.logger << "respawned\n";
	});

	COMMAND_ARGS("save_pos")
	{
		if(args.size() != 1)
		{
			game.console.error_logger << "Usage: save_pos <string: filename>\n";
			return;
		}
		string save_name = args[0];
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
		string save_name = args[0];
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
		std::ifstream file("scripts/" + args[0]);
		if(!file.is_open())
		{
			game.console.error_logger << "script not found: " << args[0] << "\n";
			return;
		}
		string line;
		while(std::getline(file, line))
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
		string part = args[0];
		double value = std::stod(args[1]);
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
			std::time_t time = std::time(nullptr);
			std::tm t = *std::localtime(&time);
			std::stringstream ss;
			ss << t.tm_year + 1900 << "-";
			ss << std::setfill('0') << std::setw(2) << t.tm_mon + 1 << "-";
			ss << std::setfill('0') << std::setw(2) << t.tm_mday << " ";
			ss << std::setfill('0') << std::setw(2) << t.tm_hour << ":";
			ss << std::setfill('0') << std::setw(2) << t.tm_min << ":";
			ss << std::setfill('0') << std::setw(2) << t.tm_sec;
			ss << " (" << time << ").png";
			filename = ss.str();
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

	COMMAND("wireframe")
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

		double value = std::stod(args[0]);
		if(args[0][0] == '+' || args[0][0] == '-')
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
		game.render_distance += 1;
		game.console.logger << "render distance: " << game.render_distance << "\n";
	});
	COMMAND("render_distance--")
	{
		game.render_distance -= 1;
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
		block_type_id_t i = static_cast<block_type_id_t>(block_type);
		i = (i + 1) % BlockType_COUNT;
		if(i < 2)
		{
			i = 2;
		}
		block_type = static_cast<BlockType>(i);
		game.console.logger << "block type: " << i << "\n";
	});
	COMMAND("block_type--")
	{
		block_type_id_t i = static_cast<block_type_id_t>(block_type);
		if(i == 2)
		{
			i = BlockType_COUNT - 1;
		}
		else
		{
			i = (i - 1) % BlockType_COUNT;
		}
		block_type = static_cast<BlockType>(i);
		game.console.logger << "block type: " << i << "\n";
	});

	COMMAND("nazi")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		Position::BlockInWorld start_pos = game.hovered_block->adjacent();
		const BlockInWorld_type ysize = 9;
		const BlockInWorld_type xsize = 9;
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
		for(BlockInWorld_type x = 0; x < xsize; ++x)
		{
			for(BlockInWorld_type y = ysize - 1; y >= 0; --y)
			{
				for(BlockInWorld_type z = 0; z < 1; ++z)
				{
					Position::BlockInWorld block_pos(x, y, z);
					block_pos += start_pos;
					block_type_id_t block_id = nazi[y][x];
					game.world.set_block(block_pos, Block::Block(block_id));
				}
			}
		}
	});

	#undef COMMAND_ARGS
	#undef COMMAND
	#undef COMMAND_
}
