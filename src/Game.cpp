#include "Game.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Coords.hpp"
#include "Gfx.hpp"
#include "console/command_test.hpp"
#include "physics/Raytracer.hpp"
#include "physics/RaytraceHit.hpp"

Game* Game::instance = nullptr;
bool Game::debug = false;

Game::Game(GLFWwindow* window)
	:
	window(window),
	hovered_block(nullptr),
	cam(window),
	delta_time(0),
	fps(144),
	keybinder(&this->console)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_test_commands(this);
	this->add_commands();
	this->bind_keys();
}

Game::~Game()
{
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	Gfx::set_cam_view(this->cam);
	this->player.step(this->delta_time);
	this->phys.step();
	this->find_hovered_block(Gfx::projection_matrix, Gfx::view_matrix);

	this->world.render_chunks();
	this->gui.draw();

	glfwSwapBuffers(window);
	glfwPollEvents();

	this->delta_time = fps.enforceFPS();

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	Position::BlockInWorld bwp(this->cam.position.x, this->cam.position.y, this->cam.position.z);
	Position::ChunkInWorld cp(bwp);
	Position::BlockInChunk bcp(bwp);
	ss << " | bwp" << bwp;
	ss << " | cp" << cp;
	ss << " | bcp" << bcp;
	ss << " | pos(" << this->player.pos.x << "," << this->player.pos.y << "," << this->player.pos.z << ")";
	glfwSetWindowTitle(window, ss.str().c_str());
}

void Game::screenshot(const std::string& filename)
{
	std::cout << "saving screenshot to " << filename << "\n";
	std::unique_ptr<GLubyte[]> pixels = std::make_unique<GLubyte[]>(3 * Gfx::width * Gfx::height);
	glReadPixels(0, 0, Gfx::width, Gfx::height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), Gfx::width, Gfx::height, true);
}

void Game::keypress(int key, int scancode, int action, int mods)
{
	this->keybinder.keypress(key, action);
	this->cam.keypress(key, action);
}

void Game::find_hovered_block(const glm::mat4& projection_matrix, const glm::mat4& view_matrix)
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	Raytracer::ScreenPosToWorldRay(
		Gfx::width / 2, Gfx::height / 2,
		Gfx::width, Gfx::height,
		glm::dmat4(view_matrix),
		glm::dmat4(projection_matrix),
		out_origin,
		out_direction
	);

	this->hovered_block = Raytracer::raycast(this->world, out_origin, out_direction, 8);
	if(this->hovered_block != nullptr)
	{
		Gfx::draw_cube_outline(this->hovered_block->pos);
	}
}

void Game::add_commands()
{
	Console* console = &this->console;

	// TODO: less copy/paste
	this->commands.emplace_back(console, "+forward", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_forward(true);
	});
	this->commands.emplace_back(console, "-forward", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_forward(false);
	});
	this->commands.emplace_back(console, "+backward", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_backward(true);
	});
	this->commands.emplace_back(console, "-backward", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_backward(false);
	});
	this->commands.emplace_back(console, "+left", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_left(true);
	});
	this->commands.emplace_back(console, "-left", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_left(false);
	});
	this->commands.emplace_back(console, "+right", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_right(true);
	});
	this->commands.emplace_back(console, "-right", [game=this](const std::vector<std::string>& args)
	{
		game->player.move_right(false);
	});

	this->commands.emplace_back(console, "jump", [game=this](const std::vector<std::string>& args)
	{
		game->player.jump();
	});
	this->commands.emplace_back(console, "noclip", [game=this](const std::vector<std::string>& args)
	{
		game->player.toggle_noclip();
	});
	this->commands.emplace_back(console, "respawn", [game=this](const std::vector<std::string>& args)
	{
		game->player.reset_position();
	});

	this->commands.emplace_back(console, "save_pos", [game=this](const std::vector<std::string>& args)
	{
		std::string save_name = "cam_pos";
		if(args.size() >= 1)
		{
			save_name = args[0];
		}
		std::ofstream streem(save_name);
		streem << game->player.pos.x << " " << game->player.pos.y << " " << game->player.pos.z << " ";
		streem << game->player.rot.x << " " << game->player.rot.y << " " << game->player.rot.z;
		streem.flush();
	});
	this->commands.emplace_back(console, "load_pos", [game=this](const std::vector<std::string>& args)
	{
		std::string save_name = "cam_pos";
		if(args.size() >= 1)
		{
			save_name = args[0];
		}
		std::ifstream streem(save_name);
		streem >> game->player.pos.x;
		streem >> game->player.pos.y;
		streem >> game->player.pos.z;
		streem >> game->cam.rotation.x;
		streem >> game->cam.rotation.y;
		streem >> game->cam.rotation.z;
	});
}

void Game::bind_keys()
{
	this->console.run_line("bind w +forward");
	this->console.run_line("bind s +backward");
	this->console.run_line("bind a +left");
	this->console.run_line("bind d +right");
	this->console.run_line("bind r respawn");
	this->console.run_line("bind space jump");
	this->console.run_line("bind l noclip");
	this->console.run_line("bind n nazi");
	this->console.run_line("bind ; save_pos");
	this->console.run_line("bind p load_pos");
	this->console.run_line("bind k +test");
}