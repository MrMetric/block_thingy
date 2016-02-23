#pragma once

#include <cstdint>
#include <map>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <graphics/OpenGL/Shader.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include "BlockType.hpp"

struct GLFWwindow;
class Camera;
namespace Position
{
	struct BlockInWorld;
}

class Gfx
{
	public:
		explicit Gfx(GLFWwindow* window);
		~Gfx();

		GLFWwindow* window;
		uint_fast32_t width;
		uint_fast32_t height;
		glm::mat4 projection_matrix;
		glm::mat4 view_matrix;

		GLuint vertex_array;
		glm::mat4 matriks;
		GLfloat* matriks_ptr;

		std::map<BlockType, Shader> block_shaders;

		Shader s_lines;
		Shader s_crosshair;

		VertexBuffer outline_vbo;

		bool cull_face;

		static GLFWwindow* init_glfw();
		static void uninit_glfw(GLFWwindow* window);
		void opengl_setup();
		void toggle_cull_face();
		void update_framebuffer_size(GLsizei width, GLsizei height);
		void set_cam_view(const Camera& cam);
		void draw_cube_outline(Position::BlockInWorld pos, const glm::vec4& color);
		const Shader& get_block_shader(BlockType) const;

		#ifdef USE_LIBPNG
		static void write_png_RGB(const char* filename, uint8_t* buf, uint_fast32_t width, uint_fast32_t height, bool reverse_rows = false);
		#endif

	private:
		void update_projection_matrix();
};
