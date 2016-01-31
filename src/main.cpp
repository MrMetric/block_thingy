#include <cstdint>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Block.hpp"
#include "Coords.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "physics/RaytraceHit.hpp"

#include <valgrind/callgrind.h>

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
#define printOpenGLError() printOglError(__FILE__, __LINE__)
void printOglError(const std::string& file, int line)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR)
	{
		std::cout << "glError in file " << file << " @ line " << line << ": " << Util::gl_error_string(glErr) << "\n";
	}
}

static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW error " << error << ": " << description << "\n";
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Game::instance->player.keypress(key, scancode, action, mods);
	Game::instance->cam.keypress(key, action);
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		else if(key == GLFW_KEY_F2)
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
			try
			{
				Game::instance->screenshot(ss.str());
			}
			catch(const std::runtime_error& e)
			{
				std::cerr << "error saving screenshot: " << e.what() << "\n";
			}
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	if(width < 0 || height < 0)
	{
		// TODO: throw exception
		std::cerr << "invalid framebuffer size: " << width << "×" << height << "\n";
	}
	Gfx::width = uint_fast32_t(width);
	Gfx::height = uint_fast32_t(height);
	glViewport(0, 0, width, height);

	Gfx::update_framebuffer_size();
	Game::instance->gui.update_framebuffer_size();

	// TODO: update camera
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game::instance->cam.handleMouseMove(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(action == GLFW_PRESS)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if(Game::instance->hovered_block != nullptr)
			{
				auto break_pos = Game::instance->hovered_block->pos;
				Game::instance->world.set_block(break_pos, nullptr);
				std::cout << "broke " << break_pos << "\n";
			}
		}
		else if(button == GLFW_MOUSE_BUTTON_RIGHT)
		{
			if(Game::instance->hovered_block != nullptr)
			{
				Position::BlockInWorld pos = Game::instance->hovered_block->adjacent();
				if(Game::instance->player.can_place_block_at(pos))
				{
					Block* block = new Block(1);
					Game::instance->world.set_block(pos, block);
				}
			}
		}
	}
}

int main()
{
	glfwSetErrorCallback(error_callback);

	std::cout << "Compiled with GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << "\n";
	std::cout << "Running with GLFW " << glfwGetVersionString() << "\n";

	Gfx::init_glfw();

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;
	std::cout << "window size: " << width << "×" << height << "\n";
	GLFWwindow* window = glfwCreateWindow(width, height, "Super Chuckward Engine", nullptr, nullptr);
	if(!window)
	{
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwGetFramebufferSize(window, &width, &height);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);

	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glewExperimental = GL_TRUE;
	GLenum glew = glewInit(); printOpenGLError();
	if(glew != GLEW_OK)
	{
		std::cerr << "Error initializing GLEW: " << glewGetErrorString(glew) << "\n";
		return 1;
	}
	else
	{
		std::cout << "INFO: GLEW initialized\n";
	}

	Game game(window);
	game.cam.keypress('P', GLFW_RELEASE); // load cam_pos

	Game::instance->player.pos = Game::instance->cam.position;
	Game::instance->player.pos.y -= Game::instance->player.eye_height;

	Gfx::opengl_setup(); printOpenGLError();

	framebuffer_size_callback(window, width, height); printOpenGLError();

	glClearColor(0.0, 0.0, 0.5, 0.0);

	Game::debug = true;
	std::cout << "starting main loop\n";
	CALLGRIND_START_INSTRUMENTATION;
	while(!glfwWindowShouldClose(window))
	{
		game.draw(); printOpenGLError();
	}
	CALLGRIND_STOP_INSTRUMENTATION;
	CALLGRIND_DUMP_STATS;
	Gfx::opengl_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}