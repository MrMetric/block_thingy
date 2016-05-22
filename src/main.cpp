#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"

#include "std_make_unique.hpp"
#include "types/window_size_t.hpp"

using std::cerr;
using std::cout;
using std::string;
using std::unique_ptr;

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
void printOglError(const string& file, const int line)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR)
	{
		cout << "glError in file " << file << " @ line " << line << ": " << Util::gl_error_string(glErr) << "\n";
	}
}
#define printOpenGLError() printOglError(__FILE__, __LINE__)

static void error_callback(const int error, const char* description)
{
	cerr << "GLFW error " << error << ": " << description << "\n";
}

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		Util::change_directory(argv[1]);
	}

	glfwSetErrorCallback(error_callback);

	cout << "Compiled with GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << "\n";
	cout << "Running with GLFW " << glfwGetVersionString() << "\n";

	GLFWwindow* window = Gfx::init_glfw();
	if(window == nullptr)
	{
		return EXIT_FAILURE;
	}

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		cerr << "Error loading GLAD\n";
		return EXIT_FAILURE;
	}
	cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << " loaded\n";
	if(!GLAD_GL_ARB_direct_state_access)
	{
		cerr << "Required OpenGL extension not found: GL_ARB_direct_state_access\n";
		return EXIT_FAILURE;
	}
	if(!GLAD_GL_ARB_separate_shader_objects)
	{
		cerr << "Required OpenGL extension not found: GL_ARB_separate_shader_objects\n";
		return EXIT_FAILURE;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	static unique_ptr<Game> game = std::make_unique<Game>(window, window_size_t(width, height)); printOpenGLError();

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		game->update_framebuffer_size(window_size_t(width, height));
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

	cout << "starting main loop\n";
	while(!glfwWindowShouldClose(window))
	{
		game->draw(); printOpenGLError();
	}

	game.reset(); // destruct
	printOpenGLError();

	Gfx::uninit_glfw(window);

	return EXIT_SUCCESS;
}
