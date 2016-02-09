#include <cstdint>
#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"

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

	GLFWwindow* window = Gfx::init_glfw();
	if(window == nullptr)
	{
		return 1;
	}

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

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Error loading GLAD\n";
		return 1;
	}
	std::cout << "OpenGL Version " << GLVersion.major << "." << GLVersion.minor << " loaded\n";

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	game = std::make_unique<Game>(window, width, height); printOpenGLError();

	std::cout << "starting main loop\n";
	while(!glfwWindowShouldClose(window))
	{
		game->draw(); printOpenGLError();
	}

	game.reset(); // destruct
	printOpenGLError();

	Gfx::uninit_glfw(window);

	return 0;
}
