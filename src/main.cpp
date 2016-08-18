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
	const GLenum glErr = glGetError();
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
	// TODO: put this somewhere else
	static_assert(GL_TRUE, "GL_TRUE is not true");
	static_assert(!GL_FALSE, "GL_FALSE is not false");

	if(argc > 1)
	{
		Util::change_directory(argv[1]);
	}

	glfwSetErrorCallback(error_callback);

	cout << "Compiled with GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION << "\n";
	cout << "Running with GLFW " << glfwGetVersionString() << "\n";

	GLFWwindow* window = Gfx::init_glfw();
	Gfx gfx(window); printOpenGLError();

	static unique_ptr<Game> game = std::make_unique<Game>(gfx);

	cout << "starting main loop\n";
	while(!glfwWindowShouldClose(game->gfx.window))
	{
		game->draw(); printOpenGLError();
	}

	game->gfx.uninit_glfw();
	game.reset(); // destruct
	printOpenGLError();

	return EXIT_SUCCESS;
}
