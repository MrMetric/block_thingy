#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Coords.hpp"
#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "physics/RaytraceHit.hpp"

#include <valgrind/callgrind.h>

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
void printOglError(const std::string& file, int line)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR)
	{
		std::cout << "glError in file " << file << " @ line " << line << ": " << Util::gl_error_string(glErr) << "\n";
	}
}
#define printOpenGLError() printOglError(__FILE__, __LINE__)

static void error_callback(int error, const char* description)
{
	std::cerr << "GLFW error " << error << ": " << description << "\n";
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Game::instance->keypress(key, scancode, action, mods);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	if(width < 0 || height < 0)
	{
		// TODO: throw exception
		std::cerr << "invalid framebuffer size: " << width << "×" << height << "\n";
	}
	Game::instance->gfx.width = uint_fast32_t(width);
	Game::instance->gfx.height = uint_fast32_t(height);
	glViewport(0, 0, width, height);

	Game::instance->gfx.update_framebuffer_size();
	Game::instance->gui.update_framebuffer_size();

	// TODO: update camera
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game::instance->cam.handleMouseMove(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Game::instance->mousepress(button, action, mods);
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

	Game game(window); printOpenGLError();

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
	game.gfx.opengl_cleanup(); printOpenGLError();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}