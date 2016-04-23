#pragma once

#include <cstdint>
#include <map>

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <graphics/OpenGL/ShaderProgram.hpp>
#include <graphics/OpenGL/VertexArray.hpp>
#include <graphics/OpenGL/VertexBuffer.hpp>

#include "BlockType.hpp"

#include <types/window_size_t.hpp>

struct GLFWwindow;

class EventManager;
namespace Position
{
	struct BlockInWorld;
}

class Gfx
{
	public:
		Gfx(GLFWwindow* window, EventManager& event_manager);

		Gfx(Gfx&&) = delete;
		Gfx(const Gfx&) = delete;
		void operator=(const Gfx&) = delete;

		GLFWwindow* window;
		window_size_t window_size;
		glm::dmat4 projection_matrix;
		glm::dmat4 view_matrix_physical;
		glm::dmat4 view_matrix_graphical;

		VertexArray vertex_array;
		glm::dmat4 matriks;

		std::map<BlockType, ShaderProgram> block_shaders;

		ShaderProgram s_lines;

		VertexBuffer outline_vbo;

		bool cull_face;

		static GLFWwindow* init_glfw();
		static void uninit_glfw(GLFWwindow* window);
		void opengl_setup();
		void toggle_cull_face();
		void set_camera_view(const glm::dvec3& position, const glm::dvec3& rotation);
		void draw_cube_outline(const Position::BlockInWorld&, const glm::vec4& color);
		const ShaderProgram& get_block_shader(BlockType) const;

		#ifdef USE_LIBPNG
		static void write_png_RGB(const char* filename, uint8_t* buf, uint_fast32_t width, uint_fast32_t height, bool reverse_rows = false);
		#endif

	private:
		void update_projection_matrix();
};
