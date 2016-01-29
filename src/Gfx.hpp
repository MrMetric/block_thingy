#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>

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

		static void init_glfw();
		static void opengl_setup();
		static void opengl_cleanup();
		static void update_framebuffer_size();
		static void set_cam_view();

		static void print_mat4(const glm::dmat4& mat);
		static void print_mat4(const glm::mat4& mat);

	private:
		Gfx(){}

		static void update_projection_matrix();
};