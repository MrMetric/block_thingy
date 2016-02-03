#include "Game.hpp"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Block.hpp"
#include "Camera.hpp"
#include "Coords.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Phys.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "console/command_test.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "gui/GUI.hpp"
#include "physics/Raytracer.hpp"
#include "physics/RaytraceHit.hpp"

Game* Game::instance = nullptr;
bool Game::debug = false;

Game::Game(GLFWwindow* window)
	:
	window(window),
	hovered_block(nullptr),
	cam(window),
	gfx(window),
	world(this->gfx.vs_cube_pos_mod),
	delta_time(0),
	fps(144),
	keybinder(&this->console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_test_commands(this);
	this->add_commands();
	this->console.run_line("exec binds");
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	this->gfx.set_cam_view(this->cam);

	this->player.rot = this->cam.rotation;
	this->player.step(this->delta_time);
	this->cam.position = this->player.pos;
	this->cam.position.y += this->player.eye_height;

	this->phys.step();
	this->find_hovered_block(this->gfx.projection_matrix, this->gfx.view_matrix);

	this->world.render_chunks();
	this->gui.draw();

	glfwSwapBuffers(window);
	glfwPollEvents();

	this->delta_time = fps.enforceFPS();

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	Position::BlockInWorld bwp(this->player.pos);
	Position::ChunkInWorld cp(bwp);
	Position::BlockInChunk bcp(bwp);
	ss << " | player.pos(" << glm::to_string(this->player.pos) << ")";
	ss << " | block" << bwp;
	ss << " | chunk" << cp;
	ss << " | chunkblock" << bcp;
	glfwSetWindowTitle(window, ss.str().c_str());
}

void Game::screenshot(const std::string& filename)
{
	std::cout << "saving screenshot to " << filename << "\n";
	std::unique_ptr<GLubyte[]> pixels = std::make_unique<GLubyte[]>(3 * this->gfx.width * this->gfx.height);
	glReadPixels(0, 0, this->gfx.width, this->gfx.height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), this->gfx.width, this->gfx.height, true);
}

void Game::keypress(int key, int scancode, int action, int mods)
{
	this->keybinder.keypress(key, action);
}

void Game::mousepress(int button, int action, int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(this->hovered_block != nullptr)
			{
				auto break_pos = this->hovered_block->pos;
				this->world.set_block(break_pos, Block());
			}
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(this->hovered_block != nullptr)
			{
				Position::BlockInWorld pos = this->hovered_block->adjacent();
				if(this->player.can_place_block_at(pos))
				{
					this->world.set_block(pos, Block(1));
				}
			}
		}
	}
}

void Game::find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	Raytracer::ScreenPosToWorldRay(
		this->gfx.width / 2, this->gfx.height / 2,
		this->gfx.width, this->gfx.height,
		glm::dmat4(view_matrix),
		glm::dmat4(projection_matrix),
		out_origin,
		out_direction
	);

	this->hovered_block = Raytracer::raycast(this->world, out_origin, out_direction, 8);
	if(this->hovered_block != nullptr)
	{
		this->gfx.draw_cube_outline(this->hovered_block->pos);
	}
}

void Game::add_commands()
{
	Console* console = &this->console;

	this->commands.emplace_back(console, "quit", [window=this->window]()
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	});

	// TODO: less copy/paste
	this->commands.emplace_back(console, "+forward", [game=this]()
	{
		game->player.move_forward(true);
	});
	this->commands.emplace_back(console, "-forward", [game=this]()
	{
		game->player.move_forward(false);
	});
	this->commands.emplace_back(console, "+backward", [game=this]()
	{
		game->player.move_backward(true);
	});
	this->commands.emplace_back(console, "-backward", [game=this]()
	{
		game->player.move_backward(false);
	});
	this->commands.emplace_back(console, "+left", [game=this]()
	{
		game->player.move_left(true);
	});
	this->commands.emplace_back(console, "-left", [game=this]()
	{
		game->player.move_left(false);
	});
	this->commands.emplace_back(console, "+right", [game=this]()
	{
		game->player.move_right(true);
	});
	this->commands.emplace_back(console, "-right", [game=this]()
	{
		game->player.move_right(false);
	});

	this->commands.emplace_back(console, "jump", [game=this]()
	{
		game->player.jump();
	});
	this->commands.emplace_back(console, "noclip", [game=this]()
	{
		game->player.toggle_noclip();
	});
	this->commands.emplace_back(console, "respawn", [game=this]()
	{
		game->player.respawn();
	});

	this->commands.emplace_back(console, "save_pos", [game=this](const std::vector<std::string>& args)
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ofstream streem(save_name);
		streem << game->player.pos.x << " " << game->player.pos.y << " " << game->player.pos.z << " ";
		streem << game->player.rot.x << " " << game->player.rot.y << " " << game->player.rot.z;
		streem.flush();
	});
	this->commands.emplace_back(console, "load_pos", [game=this](const std::vector<std::string>& args)
	{
		if(args.size() != 1)
		{
			// print usage
			return;
		}
		std::string save_name = args[0];
		std::ifstream streem(save_name);
		streem >> game->player.pos.x;
		streem >> game->player.pos.y;
		streem >> game->player.pos.z;
		streem >> game->cam.rotation.x;
		streem >> game->cam.rotation.y;
		streem >> game->cam.rotation.z;
	});

	this->commands.emplace_back(console, "exec", [console=console](const std::vector<std::string>& args)
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

	this->commands.emplace_back(console, "cam.rot", [cam=&this->cam](const std::vector<std::string>& args)
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

	this->commands.emplace_back(console, "screenshot", [game=this](const std::vector<std::string>& args)
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
}