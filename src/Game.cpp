#include "Game.hpp"

#include <iostream>
#include <memory>

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
	this->gui.draw_crosshair();

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
	std::unique_ptr<GLubyte> pixels = std::make_unique<GLubyte>(3 * Gfx::width * Gfx::height);
	glReadPixels(0, 0, Gfx::width, Gfx::height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), Gfx::width, Gfx::height, true);
}

void Game::keypress(int key, int scancode, int action, int mods)
{
	bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
	bool released = (action == GLFW_RELEASE);

	if(pressed)
	{
		this->keybinder.keypress(key);
	}
	this->player.keypress(key, scancode, action, mods);
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
	this->commands.emplace_back(&this->console, "jump", [game=this](const std::vector<std::string>& args)
	{
		game->player.jump();
	});

	this->commands.emplace_back(&this->console, "noclip", [game=this](const std::vector<std::string>& args)
	{
		game->player.toggle_noclip();
	});

	this->commands.emplace_back(&this->console, "respawn", [game=this](const std::vector<std::string>& args)
	{
		game->player.reset_position();
	});
}

void Game::bind_keys()
{
	this->keybinder.bind_key(GLFW_KEY_R, "respawn");
	this->keybinder.bind_key(GLFW_KEY_SPACE, "jump");
	this->keybinder.bind_key(GLFW_KEY_L, "noclip");
	this->keybinder.bind_key(GLFW_KEY_N, "nazi");
}