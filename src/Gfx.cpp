#include "Gfx.hpp"

#include <cstdio>							// C FILE stuff (for libpng use)
#include <cstring>							// strerror
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <graphics/OpenGL/Shader.hpp>
#include <graphics/OpenGL/VertexArray.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>			// glm::radians
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>		// glm::perspective
#include <glm/gtx/transform.hpp>			// glm::rotate, glm::translate

#ifdef USE_LIBPNG
#include <png.h>
#endif

#include "BlockType.hpp"
#include "Camera.hpp"
#include "Cube.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "position/BlockInWorld.hpp"

Gfx::Gfx(GLFWwindow* window, EventManager& event_manager)
	:
	s_lines("shaders/lines"),
	window(window)
{
	width = 0;
	height = 0;

	opengl_setup();

	event_manager.add_handler(EventType::window_size_change, [self=this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);
		self->update_framebuffer_size(e.width, e.height);
	});
}

GLFWwindow* Gfx::init_glfw()
{
	if(!glfwInit())
	{
		throw std::runtime_error("glfwInit() failed");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int width = mode->width * 3 / 4;
	int height = mode->height * 3 / 4;
	std::cout << "window size: " << width << "×" << height << "\n";
	GLFWwindow* window = glfwCreateWindow(width, height, "Baby's First Voxel Engine", nullptr, nullptr);
	if(!window)
	{
		glfwTerminate();
		return nullptr;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);

	return window;
}

void Gfx::uninit_glfw(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Gfx::opengl_setup()
{
	glClearColor(0.0, 0.0, 0.5, 0.0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	cull_face = true;
	glCullFace(GL_BACK);

	glBindVertexArray(vertex_array.get_name());

	block_shaders.emplace(BlockType::test, "shaders/block/test");
	block_shaders.emplace(BlockType::dots, "shaders/block/dots");
	block_shaders.emplace(BlockType::eye, "shaders/block/eye");

	GLfloat lineWidthRange[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL aliased line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";

	glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL smooth line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";
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

void Gfx::update_framebuffer_size(GLsizei width, GLsizei height)
{
	this->width = static_cast<uint_fast32_t>(width);
	this->height = static_cast<uint_fast32_t>(height);
	glViewport(0, 0, width, height);
	update_projection_matrix();
}

void Gfx::update_projection_matrix()
{
	const GLfloat fov  = 45.0f;
	const GLfloat near = 0.01f;
	const GLfloat far  = 1500.0f;
	const GLfloat aspect_ratio = (width > height) ? float(width)/float(height) : float(height)/float(width);
	projection_matrix = glm::perspective(fov, aspect_ratio, near, far);
}

void Gfx::set_cam_view(const Camera& cam)
{
	view_matrix = glm::dmat4(1);
	view_matrix *= glm::rotate(glm::radians(cam.rotation.x), glm::dvec3(1, 0, 0));
	view_matrix *= glm::rotate(glm::radians(cam.rotation.y), glm::dvec3(0, 1, 0));
	view_matrix *= glm::rotate(glm::radians(cam.rotation.z), glm::dvec3(0, 0, 1));
	view_matrix *= glm::translate(-1.0 * cam.position);
	matriks = glm::mat4(projection_matrix * view_matrix);
}

// TODO: use GL_LINES
void Gfx::draw_cube_outline(Position::BlockInWorld pos, const glm::vec4& color)
{
	GLfloat vertexes[16 * 3];
	uint_fast16_t o1 = 0;
	// damn thing is not Eulerian
	// TODO: determine shortest path
	GLuint elements[] = {
		0, 1, 3, 2, 0,
		4, 5, 7, 6, 4,
		5, 1, 3, 7, 6, 2
	};
	for(uint_fast8_t e = 0; e < 16; ++e)
	{
		uint_fast32_t o2 = 3 * elements[e];
		for(uint_fast8_t i = 0; i < 3; ++i)
		{
			vertexes[o1 + i] = Cube::cube_vertex[o2 + i] + pos[i];
		}
		o1 += 3;
	}

	outline_vbo.data(sizeof(vertexes), vertexes, GL_DYNAMIC_DRAW);

	glUseProgram(s_lines.get_name());
	s_lines.uniformMatrix4fv("matriks", matriks);
	s_lines.uniform4fv("color", color);

	vertex_array.attrib(0, true);
	glBindBuffer(GL_ARRAY_BUFFER, outline_vbo.get_name());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_LINE_STRIP, 0, 16);
	vertex_array.attrib(0, false);
}

const Shader& Gfx::get_block_shader(BlockType type) const
{
	auto i = block_shaders.find(type);
	if(i != block_shaders.end())
	{
		return i->second;
	}
	return block_shaders.at(BlockType::test);
}

#ifdef USE_LIBPNG
void Gfx::write_png_RGB(const char* filename, uint8_t* buf, uint_fast32_t width, uint_fast32_t height, bool reverse_rows)
{
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(png_ptr == nullptr)
	{
		throw std::runtime_error("png_create_write_struct returned null");
	}
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == nullptr)
	{
		png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png_ptr, nullptr);
		throw std::runtime_error("png_create_info_struct returned null");
	}
	FILE* fp = fopen(filename, "wb");
	if(fp == nullptr)
	{
		throw std::runtime_error(std::string("error opening png file: ") + strerror(errno));
	}
	png_init_io(png_ptr, fp);
	const int bit_depth = 8;
	png_set_IHDR(png_ptr, info_ptr, static_cast<png_uint_32>(width), static_cast<png_uint_32>(height), bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);
	uint_fast32_t rowsize = 3 * width * sizeof(png_byte);
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
