#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"

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

static std::unique_ptr<Game> game;

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
	glfwGetFramebufferSize(window, &width, &height);

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		game->update_framebuffer_size(width, height);
	});
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		game->keypress(key, scancode, action, mods);
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
	{
		game->mousepress(button, action, mods);
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y)
	{
		game->mousemove(x, y);
	});

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);

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

	game = std::make_unique<Game>(window, width, height); printOpenGLError();

	glClearColor(0.0, 0.0, 0.5, 0.0);

	Game::debug = true;
	std::cout << "starting main loop\n";
	CALLGRIND_START_INSTRUMENTATION;
	while(!glfwWindowShouldClose(window))
	{
		game->draw(); printOpenGLError();
	}
	CALLGRIND_STOP_INSTRUMENTATION;
	CALLGRIND_DUMP_STATS;
	game->gfx.opengl_cleanup(); printOpenGLError();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}