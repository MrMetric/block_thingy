#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

#include <easylogging++/easylogging++.hpp>
INITIALIZE_EASYLOGGINGPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"
#include "plugin/PluginManager.hpp"
#include "util/compiler_info.hpp"
#include "util/demangled_name.hpp"

#include "std_make_unique.hpp"

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
	LOG(ERROR) << "uncaught exception (" << Util::demangled_name(error) << ")\n"
			   << "  what():  " << error.what();
}

static void error_callback(const int error, const char* description)
{
	LOG(ERROR) << "GLFW error " << error << ": " << description;
}

int main(int argc, char** argv)
{
	std::cout << std::boolalpha;
	std::cerr << std::boolalpha;
	START_EASYLOGGINGPP(argc, argv);

	GLFWwindow* window;

	try
	{

	// TODO: put this somewhere else
	static_assert(GL_TRUE, "GL_TRUE is not true");
	static_assert(!GL_FALSE, "GL_FALSE is not false");

	#ifdef _WIN32
	CodePageHandler cp(CP_UTF8);
	#endif

	#ifdef DEBUG_BUILD
	LOG(INFO) << "This is a debug build";
	#endif

	const string compiler_info = Util::compiler_info();
	if(compiler_info != "")
	{
		LOG(DEBUG) << "Compiled by " << compiler_info;
	}

	if(argc > 1)
	{
		Util::change_directory(argv[1]);
	}

	glfwSetErrorCallback(error_callback);

	LOG(DEBUG) << "Compiled with GLFW " << GLFW_VERSION_MAJOR << "." << GLFW_VERSION_MINOR << "." << GLFW_VERSION_REVISION;
	LOG(DEBUG) << "Running with GLFW " << glfwGetVersionString();

	unique_ptr<PluginManager> plugin_manager = std::make_unique<PluginManager>();
	unique_ptr<Game> game = std::make_unique<Game>();
	window = game->gfx.window;

	LOG(INFO) << "starting main loop";
	while(!glfwWindowShouldClose(game->gfx.window))
	{
		game->draw(); printOpenGLError();
	}

	game.reset(); // destruct
	printOpenGLError();

	plugin_manager.reset();

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

	Gfx::uninit_glfw(window);

	return EXIT_SUCCESS;
}
