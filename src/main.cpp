#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "util/compiler_info.hpp"
#include "util/demangled_name.hpp"

#include "std_make_unique.hpp"

using std::cerr;
using std::cout;
using std::string;
using std::unique_ptr;

#ifdef _WIN32
class CodePageHandler
{
	public:
		CodePageHandler(UINT new_cp)
		{
			old_cp = GetConsoleOutputCP();
			SetConsoleOutputCP(new_cp);
		}
		~CodePageHandler()
		{
			SetConsoleOutputCP(old_cp);
		}

	private:
		UINT old_cp;
};
#endif

static void log_exception(const std::exception& error)
{
	cerr << "uncaught exception (" << Util::demangled_name(error) << ")\n";
	cerr << "  what():  " << error.what() << "\n";
}

static void error_callback(const int error, const char* description)
{
	cerr << "GLFW error " << error << ": " << description << "\n";
}

int main(int argc, char** argv)
{
	try
	{

	// TODO: put this somewhere else
	static_assert(GL_TRUE, "GL_TRUE is not true");
	static_assert(!GL_FALSE, "GL_FALSE is not false");

	#ifdef _WIN32
	CodePageHandler cp(CP_UTF8);
	#endif

	const string compiler_info = Util::compiler_info();
	if(compiler_info != "")
	{
		cout << "Compiled by " << compiler_info << "\n";
	}

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

	}
	catch(const std::runtime_error& error)
	{
		log_exception(error);
		return EXIT_FAILURE;
	}
	catch(const std::logic_error& error)
	{
		log_exception(error);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
