#include "Gfx.hpp"

#include <cstdio>							// C FILE stuff (for libpng use)
#include <cstring>							// strerror
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include <glm/common.hpp>					// glm::mod
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>			// glm::radians
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>		// glm::perspective
#include <glm/gtx/transform.hpp>			// glm::rotate, glm::translate

#ifdef USE_LIBPNG
#include <png.h>
#endif

#include "Camera.hpp"
#include "Cube.hpp"
#include "Game.hpp"
#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/primitive.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using std::cout;
using std::string;

Gfx::Gfx(GLFWwindow* window)
	:
	window(window),
	s_lines("shaders/lines"),
	outline_vbo({3, GL_FLOAT}),
	outline_vao(outline_vbo),
	is_fullscreen(false),
	fov(75)
{
	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);
	window_size = window_size_t(width, height);

	opengl_setup();
}

void Gfx::hook_events(EventManager& event_manager)
{
	event_manager.add_handler(EventType::window_size_change, [this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);

		window_size = e.window_size;
		glViewport(0, 0, static_cast<GLsizei>(window_size.x), static_cast<GLsizei>(window_size.y));
		update_projection_matrix();
	});
}

GLFWwindow* Gfx::init_glfw()
{
	if(!glfwInit())
	{
		throw std::runtime_error("glfwInit() failed");
	}

	GLFWwindow* window = make_window(false);

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		throw new std::runtime_error("Error loading GLAD");
	}
	std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << " loaded\n";
	if(!GLAD_GL_ARB_direct_state_access)
	{
		throw new std::runtime_error("Required OpenGL extension not found: GL_ARB_direct_state_access");
	}
	if(!GLAD_GL_ARB_separate_shader_objects)
	{
		throw new std::runtime_error("Required OpenGL extension not found: GL_ARB_separate_shader_objects");
	}

	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-parameter" // window
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
	{
		Game::instance->update_framebuffer_size(window_size_t(width, height));
	});
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		Game::instance->keypress(key, scancode, action, mods);
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
	{
		Game::instance->mousepress(button, action, mods);
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y)
	{
		Game::instance->mousemove(x, y);
	});
	#pragma clang diagnostic pop

	return window;
}

void Gfx::uninit_glfw()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Gfx::opengl_setup()
{
	glClearColor(0.0, 0.0, 0.1, 0.0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	cull_face = true;
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	block_shaders.emplace(BlockType::test, "shaders/block/test");
	block_shaders.emplace(BlockType::dots, "shaders/block/dots");
	block_shaders.emplace(BlockType::eye, "shaders/block/eye");
	block_shaders.emplace(BlockType::crappy_marble, "shaders/block/crappy_marble");
	block_shaders.emplace(BlockType::white, "shaders/block/white");
	block_shaders.emplace(BlockType::black, "shaders/block/black");
	block_shaders.emplace(BlockType::light_test_red, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_green, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_blue, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_yellow, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_cyan, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_pink, "shaders/block/light_test");
	block_shaders.emplace(BlockType::light_test_white, "shaders/block/light_test");
}

void Gfx::toggle_fullscreen()
{
	is_fullscreen = !is_fullscreen;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = is_fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = is_fullscreen ? mode->height : mode->height * 3 / 4;
	cout << "window size: " << width << "×" << height << "\n";

	const auto x = (mode->width - width) / 2;
	const auto y = (mode->height - height) / 2;
	glfwSetWindowMonitor(window, is_fullscreen ? monitor : nullptr, x, y, width, height, mode->refreshRate);
}

void Gfx::toggle_cull_face()
{
	cull_face = !cull_face;
	if(cull_face)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

void Gfx::update_projection_matrix()
{
	const double near = 0.1;
	const double far  = 1500.0; // TODO: calculate from chunk render distance
	const double width = window_size.x;
	const double height = window_size.y;
	const double aspect_ratio = (width > height) ? (width / height) : (height / width);
	projection_matrix = glm::perspective(glm::radians(fov), aspect_ratio, near, far);
}

void Gfx::set_camera_view(const glm::dvec3& position, const glm::dvec3& rotation)
{
	glm::dmat4 view_matrix(1);
	view_matrix *= glm::rotate(glm::radians(rotation.x), glm::dvec3(1, 0, 0));
	view_matrix *= glm::rotate(glm::radians(rotation.y), glm::dvec3(0, 1, 0));
	view_matrix *= glm::rotate(glm::radians(rotation.z), glm::dvec3(0, 0, 1));

	view_matrix_physical = view_matrix * glm::translate(-1.0 * position);
	view_matrix_graphical = view_matrix * glm::translate(-1.0 * glm::mod(position, static_cast<double>(CHUNK_SIZE)));

	matriks = projection_matrix * view_matrix_graphical;
}

// TODO: use GL_LINES
void Gfx::draw_cube_outline(const Position::BlockInWorld& block_pos, const glm::vec4& color)
{
	const Position::ChunkInWorld chunk_pos(block_pos);
	const auto chunk_pos_graphical = chunk_pos - Position::ChunkInWorld(Position::BlockInWorld(Game::instance->camera.position));
	const Position::BlockInWorld pos(chunk_pos_graphical, Position::BlockInChunk(block_pos));

	vertex_coord_t<GLfloat> vertexes[16];
	// damn thing is not Eulerian
	// TODO: determine shortest path
	GLuint elements[] = {
		0, 1, 3, 2, 0,
		4, 5, 7, 6, 4,
		5, 1, 3, 7, 6, 2
	};
	for(uint_fast8_t e = 0; e < 16; ++e)
	{
		const uint_fast32_t o2 = 3 * elements[e];
		for(uint_fast8_t i = 0; i < 3; ++i)
		{
			vertexes[e][i] = Cube::cube_vertex[o2 + i] + pos[i];
		}
	}

	const auto usage_hint = Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw;
	outline_vbo.data(sizeof(vertexes), vertexes, usage_hint);

	glUseProgram(s_lines.get_name());
	s_lines.uniform("matriks", matriks);
	s_lines.uniform("color", color);

	outline_vao.draw(GL_LINE_STRIP, 0, 16);
}

Graphics::OpenGL::ShaderProgram& Gfx::get_block_shader(const BlockType type)
{
	auto i = block_shaders.find(type);
	if(i != block_shaders.end())
	{
		return i->second;
	}
	return block_shaders.at(BlockType::test);
}

#ifdef USE_LIBPNG
void Gfx::write_png_RGB(const char* filename, uint8_t* buf, const uint_fast32_t width, const uint_fast32_t height, const bool reverse_rows)
{
	png_struct* png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(png_ptr == nullptr)
	{
		throw std::runtime_error("png_create_write_struct returned null");
	}
	png_info* info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == nullptr)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, nullptr);
		throw std::runtime_error("png_create_info_struct returned null");
	}
	FILE* fp = fopen(filename, "wb");
	if(fp == nullptr)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		throw std::runtime_error(string("error opening png file for writing: ") + strerror(errno));
	}
	png_init_io(png_ptr, fp);
	const int bit_depth = 8;
	const png_uint_32 w = static_cast<png_uint_32>(width);
	const png_uint_32 h = static_cast<png_uint_32>(height);
	png_set_IHDR(png_ptr, info_ptr, w, h, bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	const uint_fast32_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
	if(reverse_rows)
	{
		for(uint_fast32_t y = height; y > 0; --y)
		{
			png_write_row(png_ptr, buf + (y - 1) * rowsize);
		}
	}
	else
	{
		for(uint_fast32_t y = 0; y < height; ++y)
		{
			png_write_row(png_ptr, buf + y * rowsize);
		}
	}
	png_write_end(png_ptr, info_ptr);
	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}
#endif

GLFWwindow* Gfx::make_window(bool is_fullscreen)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = is_fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = is_fullscreen ? mode->height : mode->height * 3 / 4;
	cout << "window size: " << width << "×" << height << "\n";
	GLFWwindow* window = glfwCreateWindow(width, height, "Baby's First Voxel Engine", nullptr, nullptr);
	if(window == nullptr)
	{
		glfwTerminate();
		throw new std::runtime_error("error creating window");
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);

	return window;
}
