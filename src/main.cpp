#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifdef _WIN32
#include <windows.h>
#endif

#include <easylogging++/easylogging++.hpp>
INITIALIZE_EASYLOGGINGPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "console/Console.hpp"
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
	static_assert(std::is_same<GLbyte  ,   int8_t>::value, "GLbyte is not int8_t");
	static_assert(std::is_same<GLubyte ,  uint8_t>::value, "GLubyte is not uint8_t");
	static_assert(std::is_same<GLshort ,  int16_t>::value, "GLshort is not int16_t");
	static_assert(std::is_same<GLushort, uint16_t>::value, "GLushort is not uint16_t");
	static_assert(std::is_same<GLint   ,  int32_t>::value, "GLint is not int32_t");
	static_assert(std::is_same<GLuint  , uint32_t>::value, "GLuint is not uint32_t");
	static_assert(std::is_same<GLint64 ,  int64_t>::value, "GLint64 is not int64_t");
	static_assert(std::is_same<GLuint64, uint64_t>::value, "GLuint64 is not uint64_t");
	static_assert(std::is_same<GLfloat ,    float>::value, "GLfloat is not float");
	static_assert(std::is_same<GLdouble,   double>::value, "GLdouble is not double");

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

	unique_ptr<Console> console = std::make_unique<Console>();
	// maybe Settings should have a constructor
	Settings::add_command_handlers();
	Settings::load();
	unique_ptr<PluginManager> plugin_manager = std::make_unique<PluginManager>();
	unique_ptr<Game> game = std::make_unique<Game>();
	window = game->gfx.window;

	LOG(INFO) << "starting main loop";
	while(!glfwWindowShouldClose(game->gfx.window))
	{
		game->draw();
	}

	game.reset(); // destruct
	plugin_manager.reset();
	Settings::save();
	console.reset();

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
