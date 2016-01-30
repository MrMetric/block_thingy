#include "Gfx.hpp"

#include <iostream>
#include <stdexcept>				// std::runtime_error

#include <glm/gtx/transform.hpp>	// glm::perspective
#include <glm/gtc/type_ptr.hpp>		// glm::value_ptr

#include <png.h>

#include "Game.hpp"
#include "shader_util.hpp"

GLFWwindow* Gfx::window = nullptr;
uint_fast32_t Gfx::width = 0;
uint_fast32_t Gfx::height = 0;
glm::mat4 Gfx::projection_matrix;
glm::mat4 Gfx::view_matrix;
GLuint Gfx::vertex_array = 0;
glm::mat4 Gfx::matriks;
GLfloat* Gfx::matriks_ptr = nullptr;

GLuint Gfx::sp_cube = ~GLuint(0);
GLint Gfx::vs_cube_matriks = -1;
GLint Gfx::vs_cube_pos_mod = -1;

GLuint Gfx::sp_lines = ~GLuint(0);
GLint Gfx::vs_lines_matriks = -1;
GLint Gfx::vs_lines_color = -1;

GLuint Gfx::sp_crosshair = ~GLuint(0);
GLint Gfx::vs_crosshair_matriks = -1;

void Gfx::init_glfw()
{
	if(!glfwInit())
	{
		exit(1);
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

	glGenVertexArrays(1, &Gfx::vertex_array);
	glBindVertexArray(Gfx::vertex_array);

	Gfx::sp_cube = make_program("shaders/cube");
	Gfx::vs_cube_matriks = getUniformLocation(Gfx::sp_cube, "matriks");
	Gfx::vs_cube_pos_mod = getUniformLocation(Gfx::sp_cube, "pos_mod");

	Gfx::sp_lines = make_program("shaders/lines");
	Gfx::vs_lines_matriks = getUniformLocation(Gfx::sp_lines, "matriks");
	Gfx::vs_lines_color = getUniformLocation(Gfx::sp_lines, "color");

	Gfx::sp_crosshair = make_program("shaders/crosshair");
	Gfx::vs_crosshair_matriks = getUniformLocation(Gfx::sp_crosshair, "matriks");

	GLfloat lineWidthRange[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL aliased line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";

	glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, lineWidthRange);
	std::cout << "OpenGL smooth line width range: " << lineWidthRange[0] << "," << lineWidthRange[1] << "\n";
}

void Gfx::opengl_cleanup()
{
	glDeleteVertexArrays(1, &Gfx::vertex_array);
	glDeleteProgram(Gfx::sp_cube);
	glDeleteProgram(Gfx::sp_lines);
	glDeleteProgram(Gfx::sp_crosshair);
}

void Gfx::update_framebuffer_size()
{
	Gfx::update_projection_matrix();
}

void Gfx::update_projection_matrix()
{
	const GLfloat fov  = 45.0f;
	const GLfloat near = 0.01f;
	const GLfloat far  = 1500.0f;
	const GLfloat aspect_ratio = (width > height) ? float(width)/float(height) : float(height)/float(width);
	Gfx::projection_matrix = glm::perspective(fov, aspect_ratio, near, far);
}

void Gfx::set_cam_view()
{
	glm::dmat4 view;
	view *= glm::rotate(glm::radians(Game::instance->cam.rotation.x), glm::dvec3(1, 0, 0));
	view *= glm::rotate(glm::radians(Game::instance->cam.rotation.y), glm::dvec3(0, 1, 0));
	view *= glm::rotate(glm::radians(Game::instance->cam.rotation.z), glm::dvec3(0, 0, 1));
	glm::dvec3 position = Game::instance->cam.position * -1.0;
	view *= glm::translate(position);
	glm::mat4 viewf(view);
	Gfx::matriks = Gfx::projection_matrix * viewf;
	Gfx::matriks_ptr = glm::value_ptr(Gfx::matriks);
	Gfx::view_matrix = viewf;
}

void Gfx::write_png_RGB(const char* filename, uint8_t* buf, uint32_t width, uint32_t height)
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
	for(uint_fast32_t y = 0; y < height; ++y)
	{
		png_write_row(png_ptr, buf + y * rowsize);
	}
	png_write_end(png_ptr, info_ptr);
	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void Gfx::print_mat4(const glm::dmat4& mat)
{
	std::cout << "matrix4:\n";
	const double* p = static_cast<const double*>(glm::value_ptr(mat));
	for(int x = 0; x < 4; ++x)
	{
		for(int y = 0; y < 4; ++y)
		{
			std::cout << p[y*4 + x] << ", ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

void Gfx::print_mat4(const glm::mat4& mat)
{
	const float* p = static_cast<const float*>(glm::value_ptr(mat));
	for(int x = 0; x < 4; ++x)
	{
		std::cout << "(";
		for(int y = 0; y < 4; ++y)
		{
			std::cout << p[y*4 + x];
			if(y < 3)
			{
				std::cout << ", ";
			}
		}
		std::cout << ")\n";
	}
	std::cout << "\n";
}