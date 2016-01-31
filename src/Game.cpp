#include "Game.hpp"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Coords.hpp"
#include "Gfx.hpp"
#include "console/command_test.hpp"

Game* Game::instance = nullptr;
bool Game::debug = false;

Game::Game(GLFWwindow* window)
	:
	window(window),
	cam(window),
	delta_time(0),
	fps(144)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots

	add_test_commands(this);
}

Game::~Game()
{
}

void Game::draw()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	Gfx::set_cam_view();
	this->player.step(this->delta_time);
	this->phys.step(Gfx::projection_matrix, Gfx::view_matrix);

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
	GLubyte* pixels = new GLubyte[3 * Gfx::width * Gfx::height];
	glReadPixels(0, 0, Gfx::width, Gfx::height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	Gfx::write_png_RGB(filename.c_str(), pixels, Gfx::width, Gfx::height, true);
	delete[] pixels;
}