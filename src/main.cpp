#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifdef _WIN32
	#include <windows.h>
#endif

#include <GLFW/glfw3.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "language.hpp"
#include "settings.hpp"
#include "console/Console.hpp"
#include "plugin/PluginManager.hpp"
#include "util/compiler_info.hpp"
#include "util/demangled_name.hpp"
#include "util/filesystem.hpp"
#include "util/logger.hpp"

using std::string;
using std::unique_ptr;

namespace block_thingy {

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
	LOG(ERROR) << "uncaught exception (" << util::demangled_name(error) << ")\n"
			   << "  what():  " << error.what() << '\n';
}

static void error_callback(const int error, const char* description)
{
	LOG(ERROR) << "GLFW error " << error << ": " << description << '\n';
}

}

using namespace block_thingy;

int main(const int argc, char** argv)
{
	std::cout << std::boolalpha;
	std::cerr << std::boolalpha;

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
	LOG(INFO) << "This is a debug build\n";
#endif

	const string compiler_info = util::compiler_info();
	if(compiler_info != "")
	{
		LOG(DEBUG) << "Compiled by " << compiler_info << '\n';
	}

	if(argc > 1)
	{
		fs::current_path(argv[1]);
	}
#ifdef _WIN32
	else
	{
		const string pwd = fs::current_path().string();
		const auto i = pwd.find("\\projects\\vs");
		if(i != string::npos)
		{
			const fs::path bin = fs::path(pwd.substr(0, i)) / "bin";
			LOG(INFO) << "Using detected bin path: " << bin << '\n';
			fs::current_path(bin);
		}
	}
#endif

	for(const fs::directory_entry& entry : fs::directory_iterator("lang"))
	{
		const fs::path path = entry.path();
		if(path.extension() != ".btlang")
		{
			continue;
		}
		language::load(path.stem().string(), path);
	}

	glfwSetErrorCallback(error_callback);

	LOG(DEBUG) << "Compiled with GLFW " << GLFW_VERSION_MAJOR << '.' << GLFW_VERSION_MINOR << '.' << GLFW_VERSION_REVISION << '\n';
	LOG(DEBUG) << "Running with GLFW " << glfwGetVersionString() << '\n';

	unique_ptr<Console> console = std::make_unique<Console>();
	// maybe Settings should have a constructor
	settings::add_command_handlers();
	settings::load();

	unique_ptr<PluginManager> plugin_manager = std::make_unique<PluginManager>();
	unique_ptr<game> g = std::make_unique<game>();
	window = g->gfx.window;

	LOG(DEBUG) << "starting main loop" << '\n';
	while(!glfwWindowShouldClose(g->gfx.window))
	{
		g->draw();
	}

	g = nullptr;
	plugin_manager = nullptr;
	console = nullptr;

	}
	catch(const std::exception& error)
	{
		log_exception(error);
		return EXIT_FAILURE;
	}

	Gfx::uninit_glfw(window);

	return EXIT_SUCCESS;
}
