#include "Game.hpp"

#include <png++/png.hpp>

#include "Gfx.hpp"

Game* Game::instance = nullptr;
bool Game::debug = false;

Game::Game(GLFWwindow* window)
	:
	cam(window),
	window(window),
	delta_time(0),
	fps(144)
{
	Game::instance = this;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // for screenshots
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
	std::cout << "saving " << filename << "\n";
	GLubyte* pixels = new GLubyte[3 * Gfx::width * Gfx::height];
	glReadPixels(0, 0, Gfx::width, Gfx::height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	png::image<png::rgb_pixel> image(Gfx::width, Gfx::height);
	for(uint_fast32_t y = 0; y < Gfx::height; ++y)
	{
		for(uint_fast32_t x = 0; x < Gfx::width; ++x)
		{
			uint_fast32_t index = 3 * (y * Gfx::width + x);
			image.set_pixel(x, Gfx::height - y - 1, png::rgb_pixel(pixels[index], pixels[index + 1], pixels[index + 2]));
		}
	}
	delete[] pixels;
	image.write(filename);
}