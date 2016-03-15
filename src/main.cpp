#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Poco/BinaryReader.h>
using Poco::BinaryReader;
#include <Poco/BinaryWriter.h>
using Poco::BinaryWriter;
#include <Poco/DeflatingStream.h>
#include <Poco/InflatingStream.h>

#include "Game.hpp"
#include "Gfx.hpp"
#include "Util.hpp"

#include "std_make_unique.hpp"

// http://www.lighthouse3d.com/cg-topics/error-tracking-in-opengl/
void printOglError(const std::string& file, const int line)
{
	GLenum glErr = glGetError();
	if(glErr != GL_NO_ERROR)
	{
		std::cout << "glError in file " << file << " @ line " << line << ": " << Util::gl_error_string(glErr) << "\n";
	}
}
#define printOpenGLError() printOglError(__FILE__, __LINE__)

static void error_callback(const int error, const char* description)
{
	std::cerr << "GLFW error " << error << ": " << description << "\n";
}

static std::unique_ptr<Game> game;

int main(int argc, char** argv)
{
	if(argc > 1)
	{
		Util::change_directory(argv[1]);
	}

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
	if(!Util::file_is_openable("world.gz"))
	{
		std::ofstream stdstream("world.gz", std::ios::binary);
		Poco::DeflatingOutputStream stream(stdstream, Poco::DeflatingStreamBuf::STREAM_GZIP);
		BinaryWriter writer(stream);
		writer << static_cast<uint64_t>(0);
	}
	// scope lets streams and reader destruct
	{
		std::ifstream stdstream("world.gz", std::ios::binary);
		Poco::InflatingInputStream stream(stdstream, Poco::InflatingStreamBuf::STREAM_GZIP);
		BinaryReader reader(stream);
		game = std::make_unique<Game>(window, width, height, reader); printOpenGLError();
	}

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
