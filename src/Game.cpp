#include "Game.hpp"

#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
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

#include "Block.hpp"
#include "BlockType.hpp"
#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/Event.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/RenderWorld.hpp"
#include "gui/GUI.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/RaytraceHit.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

#include "std_make_unique.hpp"

Game* Game::instance = nullptr;

Game::Game(GLFWwindow* window, const uint_fast32_t width, const uint_fast32_t height)
	:
	window(window),
	hovered_block(nullptr),
	cam(window, event_manager),
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
	keybinder(&console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(width, height);

	cam.rotation = player.rotation; // keep saved value
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gfx.set_cam_view(cam);

	player.rotation = cam.rotation;
	world.step(delta_time);
	cam.position = player.position;
	cam.position.y += player.get_eye_height();

	Position::BlockInWorld render_origin(player.position);
	RenderWorld::draw_world(world, gfx.block_shaders, gfx.matriks, render_origin, render_distance);
	find_hovered_block(gfx.projection_matrix, gfx.view_matrix);
	gui.draw(gfx);

	glfwSwapBuffers(window);
	glfwPollEvents();

	delta_time = fps.enforceFPS();

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	ss << " | player.pos(" << glm::to_string(player.position) << ")";
	Position::BlockInWorld player_block_pos(player.position);
	ss << " | block" << player_block_pos;
	ss << " | chunk" << Position::ChunkInWorld(player_block_pos);
	ss << " | chunkblock" << Position::BlockInChunk(player_block_pos);
	glfwSetWindowTitle(window, ss.str().c_str());
}

#ifdef USE_LIBPNG
void Game::screenshot(const std::string& filename)
{
	std::cout << "saving screenshot to " << filename << "\n";
	std::unique_ptr<GLubyte[]> pixels = std::make_unique<GLubyte[]>(3 * gfx.width * gfx.height);
	glReadPixels(0, 0, gfx.width, gfx.height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), gfx.width, gfx.height, true);
}
#endif

void Game::update_framebuffer_size(const uint_fast32_t width, const uint_fast32_t height)
{
	event_manager.do_event(Event_window_size_change(width, height));
}

void Game::keypress(const int key, const int scancode, const int action, const int mods)
{
	keybinder.keypress(key, action);
}

static BlockType block_type = BlockType::test;
void Game::mousepress(const int button, const int action, const int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(hovered_block != nullptr)
			{
				const Position::BlockInWorld pos = hovered_block->pos;
				if(world.get_block_const(pos).type() != BlockType::none)
				{
					world.set_block(pos, Block(BlockType::air));
					find_hovered_block(gfx.projection_matrix, gfx.view_matrix);
					//event_manager.do_event(Event_break_block(pos, face));
				}
			}
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(hovered_block != nullptr)
			{
				const Position::BlockInWorld pos = hovered_block->adjacent();
				if(world.get_block_const(pos).type() == BlockType::air && player.can_place_block_at(pos))
				{
					world.set_block(pos, Block(block_type));
					//event_manager.do_event(Event_place_block(pos, face));
				}
			}
		}
	}
}

void Game::mousemove(const double x, const double y)
{
	cam.handleMouseMove(x, y);
}

void Game::find_hovered_block(const glm::dmat4& projection_matrix, const glm::dmat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay(
		glm::vec2(gfx.width / 2.0, gfx.height / 2.0),
		glm::uvec2(gfx.width, gfx.height),
		view_matrix,
		projection_matrix,
		out_origin,
		out_direction
	);

	hovered_block = PhysicsUtil::raycast(world, out_origin, out_direction, 8);
	if(hovered_block != nullptr)
	{
		bool block_is_none = world.get_block_const(hovered_block->pos).type() == BlockType::none;
		glm::vec4 color = block_is_none ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 1, 1);
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	Console* console = &this->console;

	#define COMMAND_(name) commands.emplace_back(console, name, [game=this]
	#define COMMAND(name) COMMAND_(name)()
	#define COMMAND_ARGS(name) COMMAND_(name)(const std::vector<std::string>& args)

	COMMAND("quit")
	{
		game->world.save();
		glfwSetWindowShouldClose(game->window, GL_TRUE);
	});

	// TODO: less copy/paste
	COMMAND("+forward")
	{
		game->player.move_forward(true);
	});
	COMMAND("-forward")
	{
		game->player.move_forward(false);
	});
	COMMAND("+backward")
	{
		game->player.move_backward(true);
	});
	COMMAND("-backward")
	{
		game->player.move_backward(false);
	});
	COMMAND("+left")
	{
		game->player.move_left(true);
	});
	COMMAND("-left")
	{
		game->player.move_left(false);
	});
	COMMAND("+right")
	{
		game->player.move_right(true);
	});
	COMMAND("-right")
	{
		game->player.move_right(false);
	});
	COMMAND("+sprint")
	{
		game->player.go_faster(true);
	});
	COMMAND("-sprint")
	{
		game->player.go_faster(false);
	});

	COMMAND("jump")
	{
		game->player.jump();
	});
	COMMAND("noclip")
	{
		game->player.toggle_noclip();
	});
	COMMAND("respawn")
	{
		game->player.respawn();
	});

	COMMAND_ARGS("save_pos")
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ofstream streem(save_name);
		streem << game->player.position.x << " " << game->player.position.y << " " << game->player.position.z << " ";
		streem << game->player.rotation.x << " " << game->player.rotation.y << " " << game->player.rotation.z;
		streem.flush();
	});
	COMMAND_ARGS("load_pos")
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ifstream streem(save_name);
		streem >> game->player.position.x;
		streem >> game->player.position.y;
		streem >> game->player.position.z;
		streem >> game->cam.rotation.x;
		streem >> game->cam.rotation.y;
		streem >> game->cam.rotation.z;
	});

	COMMAND_ARGS("exec")
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::ifstream file("scripts/" + args[0]);
		std::string line;
		while(std::getline(file, line))
		{
			game->console.run_line(line);
		}
	});

	COMMAND_ARGS("cam.rot")
	{
		if(args.size() != 2)
		{
			// print usage
			return;
		}
		std::string part = args[0];
		double value = std::stod(args[1]);
		if(part == "x")
		{
			game->cam.rotation.x += value;
		}
		else if(part == "y")
		{
			game->cam.rotation.y += value;
		}
		else if(part == "z")
		{
			game->cam.rotation.z += value;
		}
		else
		{
			// error
		}
	});

	#ifdef USE_LIBPNG
	COMMAND_ARGS("screenshot")
	{
		std::string filename;
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
			// print usage
			return;
		}
		try
		{
			game->screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			std::cerr << "error saving screenshot: " << e.what() << "\n";
		}
	});
	#endif

	COMMAND("wireframe")
	{
		game->wireframe = !game->wireframe();
	});
	COMMAND("toggle_cull_face")
	{
		game->gfx.toggle_cull_face();
		std::cout << "cull face: " << (game->gfx.cull_face ? "true" : "false") << "\n";
	});

	COMMAND("render_distance++")
	{
		game->render_distance += 1;
	});
	COMMAND("render_distance--")
	{
		game->render_distance -= 1;
	});

	COMMAND("change_block_type")
	{
		block_type_id_t i = static_cast<block_type_id_t>(block_type);
		i = (i + 1) % BlockType_COUNT;
		block_type = static_cast<BlockType>(i);
		std::cout << "block type: " << i << "\n";
	});
	console->run_line("bind j change_block_type");

	COMMAND("nazi")
	{
		if(game->hovered_block == nullptr)
		{
			return;
		}

		Position::BlockInWorld start_pos = game->hovered_block->adjacent();
		const size_t ysize = 9;
		const size_t xsize = 9;
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
					game->world.set_block(block_pos, Block(block_id));
				}
			}
		}
	});

	#undef COMMAND_ARGS
	#undef COMMAND
	#undef COMMAND_
}
