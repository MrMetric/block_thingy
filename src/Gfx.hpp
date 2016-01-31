#pragma once

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>

#include "Coords.hpp"

class Camera;

class Gfx
{
	public:
		static GLFWwindow* window;
		static uint_fast32_t width;
		static uint_fast32_t height;
		static glm::mat4 projection_matrix;
		static glm::mat4 view_matrix;

		static GLuint vertex_array;
		static glm::mat4 matriks;
		static GLfloat* matriks_ptr;

		static GLuint sp_cube;
		static GLint vs_cube_matriks;
		static GLint vs_cube_pos_mod;

		static GLuint sp_lines;
		static GLint vs_lines_matriks;
		static GLint vs_lines_color;

		static GLuint sp_crosshair;
		static GLint vs_crosshair_matriks;

		static GLuint outline_vbo;

		static void init_glfw();
		static void opengl_setup();
		static void opengl_cleanup();
		static void update_framebuffer_size();
		static void set_cam_view(const Camera& cam);

		static void draw_cube_outline(Position::BlockInWorld pos, const glm::vec4& color = glm::vec4(1, 1, 1, 1));

		static void write_png_RGB(const char* filename, uint8_t* buf, uint32_t width, uint32_t height, bool reverse_rows = false);

		static void print_mat4(const glm::dmat4& mat);
		static void print_mat4(const glm::mat4& mat);

	private:
		Gfx(){}

		static void update_projection_matrix();
};