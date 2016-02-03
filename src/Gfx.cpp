#include "Gfx.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/gtx/transform.hpp>	// glm::perspective
#include <glm/gtc/type_ptr.hpp>		// glm::value_ptr

#include <png.h>

#include "Camera.hpp"
#include "Coords.hpp"
#include "Cube.hpp"
#include "shader_util.hpp"

Gfx::Gfx(GLFWwindow* window)
	:
	window(window)
{
	this->width = 0;
	this->height = 0;
	this->matriks_ptr = nullptr;

	this->opengl_setup();
}

void Gfx::init_glfw()
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
	glEnable(GL_MULTISAMPLE);
}

void Gfx::opengl_setup()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glGenVertexArrays(1, &this->vertex_array);
	glBindVertexArray(this->vertex_array);

	this->sp_cube = make_program("shaders/cube");
	this->vs_cube_matriks = getUniformLocation(this->sp_cube, "matriks");
	this->vs_cube_pos_mod = getUniformLocation(this->sp_cube, "pos_mod");

	this->sp_lines = make_program("shaders/lines");
	this->vs_lines_matriks = getUniformLocation(this->sp_lines, "matriks");
	this->vs_lines_color = getUniformLocation(this->sp_lines, "color");

	this->sp_crosshair = make_program("shaders/crosshair");
	this->vs_crosshair_matriks = getUniformLocation(this->sp_crosshair, "matriks");

	glGenBuffers(1, &this->outline_vbo);

	GLfloat lineWidthRange[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL aliased line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";

	glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL smooth line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";
}

void Gfx::opengl_cleanup()
{
	glDeleteVertexArrays(1, &this->vertex_array);
	glDeleteProgram(this->sp_cube);
	glDeleteProgram(this->sp_lines);
	glDeleteProgram(this->sp_crosshair);
	glDeleteBuffers(1, &this->outline_vbo);
}

void Gfx::update_framebuffer_size(GLsizei width, GLsizei height)
{
	this->width = width;
	this->height = height;
	glViewport(0, 0, width, height);
	this->update_projection_matrix();
}

void Gfx::update_projection_matrix()
{
	const GLfloat fov  = 45.0f;
	const GLfloat near = 0.01f;
	const GLfloat far  = 1500.0f;
	const GLfloat aspect_ratio = (width > height) ? float(width)/float(height) : float(height)/float(width);
	this->projection_matrix = glm::perspective(fov, aspect_ratio, near, far);
}

void Gfx::set_cam_view(const Camera& cam)
{
	glm::dmat4 view;
	view *= glm::rotate(glm::radians(cam.rotation.x), glm::dvec3(1, 0, 0));
	view *= glm::rotate(glm::radians(cam.rotation.y), glm::dvec3(0, 1, 0));
	view *= glm::rotate(glm::radians(cam.rotation.z), glm::dvec3(0, 0, 1));
	glm::dvec3 position = cam.position * -1.0;
	view *= glm::translate(position);
	glm::mat4 viewf(view);
	this->matriks = this->projection_matrix * viewf;
	this->matriks_ptr = glm::value_ptr(this->matriks);
	this->view_matrix = viewf;
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
			vertexes[o1 + i] = Cube::cube_vertex[o2 + i];
			if(i == 0)
			{
				vertexes[o1 + i] += pos.x;
			}
			else if(i == 1)
			{
				vertexes[o1 + i] += pos.y;
			}
			else if(i == 2)
			{
				vertexes[o1 + i] += pos.z;
			}
		}
		o1 += 3;
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->outline_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_DYNAMIC_DRAW);

	glUseProgram(this->sp_lines);
	glUniformMatrix4fv(this->vs_lines_matriks, 1, GL_FALSE, this->matriks_ptr);
	glUniform4fv(this->vs_lines_color, 1, glm::value_ptr(color));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, this->outline_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glDrawArrays(GL_LINE_STRIP, 0, 16);
	glDisableVertexAttribArray(0);
}

void Gfx::write_png_RGB(const char* filename, uint8_t* buf, uint32_t width, uint32_t height, bool reverse_rows)
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
	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
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