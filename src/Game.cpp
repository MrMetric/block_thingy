#include "Game.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Block.hpp"
#include "Camera.hpp"
#include "Coords.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "chunk/Chunk.hpp"
#include "console/command_test.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/Event.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "gui/GUI.hpp"
#include "physics/Raytracer.hpp"
#include "physics/RaytraceHit.hpp"

Game* Game::instance = nullptr;

Game::Game(GLFWwindow* window, int width, int height)
	:
	window(window),
	hovered_block(nullptr),
	cam(window),
	gfx(window),
	world(gfx.vs_cube_pos_mod),
	delta_time(0),
	fps(144),
	keybinder(&console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_test_commands(this);
	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(width, height);
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gfx.set_cam_view(cam);

	player.rotation = cam.rotation;
	player.step(delta_time);
	cam.position = player.position;
	cam.position.y += player.eye_height;

	find_hovered_block(gfx.projection_matrix, gfx.view_matrix);

	draw_world();
	gui.draw(gfx);

	glfwSwapBuffers(window);
	glfwPollEvents();

	delta_time = fps.enforceFPS();

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	Position::BlockInWorld bwp(player.position);
	Position::ChunkInWorld cp(bwp);
	Position::BlockInChunk bcp(bwp);
	ss << " | player.pos(" << glm::to_string(player.position) << ")";
	ss << " | block" << bwp;
	ss << " | chunk" << cp;
	ss << " | chunkblock" << bcp;
	glfwSetWindowTitle(window, ss.str().c_str());
}

void Game::draw_world()
{
	glUseProgram(gfx.sp_cube);
	glUniformMatrix4fv(gfx.vs_cube_matriks, 1, GL_FALSE, gfx.matriks_ptr);

	const int render_distance = 5;

	Position::ChunkInWorld cp(Position::BlockInWorld(player.position));
	Position::ChunkInWorld min(cp.x - render_distance, cp.y - render_distance, cp.z - render_distance);
	Position::ChunkInWorld max(cp.x + render_distance, cp.y + render_distance, cp.z + render_distance);
	for(int x = min.x; x <= max.x; ++x)
	{
		for(int y = min.y; y <= max.y; ++y)
		{
			for(int z = min.z; z <= max.z; ++z)
			{
				std::shared_ptr<Chunk> chunk = world.get_or_make_chunk(Position::ChunkInWorld(x, y, z));
				chunk->render();
			}
		}
	}
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

void Game::update_framebuffer_size(int width, int height)
{
	gfx.update_framebuffer_size(width, height);
	gui.update_framebuffer_size(width, height);

	// TODO: update camera
}

void Game::keypress(int key, int scancode, int action, int mods)
{
	keybinder.keypress(key, action);
}

void Game::mousepress(int button, int action, int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(hovered_block != nullptr && world.get_block(hovered_block->pos).type() != BlockType::none)
			{
				auto break_pos = hovered_block->pos;
				world.set_block(break_pos, Block(BlockType::air));
				find_hovered_block(gfx.projection_matrix, gfx.view_matrix);
				event_manager.do_event(Event(EventType::break_block));
			}
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(hovered_block != nullptr)
			{
				Position::BlockInWorld pos = hovered_block->adjacent();
				if(player.can_place_block_at(pos))
				{
					world.set_block(pos, Block(BlockType::test));
				}
			}
		}
	}
}

void Game::mousemove(double x, double y)
{
	cam.handleMouseMove(x, y);
}

void Game::find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	Raytracer::ScreenPosToWorldRay(
		gfx.width / 2, gfx.height / 2,
		gfx.width, gfx.height,
		glm::dmat4(view_matrix),
		glm::dmat4(projection_matrix),
		out_origin,
		out_direction
	);

	hovered_block = Raytracer::raycast(world, out_origin, out_direction, 8);
	if(hovered_block != nullptr)
	{
		bool block_is_none = world.get_block(hovered_block->pos).type() == BlockType::none;
		glm::vec4 color = block_is_none ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 1, 1);
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	Console* console = &this->console;

	commands.emplace_back(console, "quit", [window=window]()
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	});

	// TODO: less copy/paste
	commands.emplace_back(console, "+forward", [game=this]()
	{
		game->player.move_forward(true);
	});
	commands.emplace_back(console, "-forward", [game=this]()
	{
		game->player.move_forward(false);
	});
	commands.emplace_back(console, "+backward", [game=this]()
	{
		game->player.move_backward(true);
	});
	commands.emplace_back(console, "-backward", [game=this]()
	{
		game->player.move_backward(false);
	});
	commands.emplace_back(console, "+left", [game=this]()
	{
		game->player.move_left(true);
	});
	commands.emplace_back(console, "-left", [game=this]()
	{
		game->player.move_left(false);
	});
	commands.emplace_back(console, "+right", [game=this]()
	{
		game->player.move_right(true);
	});
	commands.emplace_back(console, "-right", [game=this]()
	{
		game->player.move_right(false);
	});

	commands.emplace_back(console, "jump", [game=this]()
	{
		game->player.jump();
	});
	commands.emplace_back(console, "noclip", [game=this]()
	{
		game->player.toggle_noclip();
	});
	commands.emplace_back(console, "respawn", [game=this]()
	{
		game->player.respawn();
	});

	commands.emplace_back(console, "save_pos", [game=this](const std::vector<std::string>& args)
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
	commands.emplace_back(console, "load_pos", [game=this](const std::vector<std::string>& args)
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

	commands.emplace_back(console, "exec", [console=console](const std::vector<std::string>& args)
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
			console->run_line(line);
		}
	});

	commands.emplace_back(console, "cam.rot", [cam=&cam](const std::vector<std::string>& args)
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
			cam->rotation.x += value;
		}
		else if(part == "y")
		{
			cam->rotation.y += value;
		}
		else if(part == "z")
		{
			cam->rotation.z += value;
		}
		else
		{
			// error
		}
	});

	#ifdef USE_LIBPNG
	commands.emplace_back(console, "screenshot", [game=this](const std::vector<std::string>& args)
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

	commands.emplace_back(console, "wireframe", []()
	{
		static bool wireframe = false; // TODO: less static
		wireframe = !wireframe;
		if(wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	});
	commands.emplace_back(console, "toggle_cull_face", [game=this]()
	{
		game->gfx.toggle_cull_face();
		std::cout << "cull face: " << (game->gfx.cull_face ? "true" : "false") << "\n";
	});
}
