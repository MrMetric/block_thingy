#pragma once

#include <map>
#include <stdint.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "graphics/Text.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "fwd/block/BlockType.hpp"

#include "types/window_size_t.hpp"

struct GLFWwindow;

class EventManager;
namespace Position
{
	struct BlockInWorld;
}

class Gfx
{
	public:
		Gfx(GLFWwindow*);

		Gfx(Gfx&&) = delete;
		Gfx(const Gfx&) = delete;
		void operator=(const Gfx&) = delete;

		GLFWwindow* window;
		window_size_t window_size;
		glm::dvec2 window_mid;
		glm::dmat4 projection_matrix;
		glm::dmat4 view_matrix_physical;
		glm::dmat4 view_matrix_graphical;

		glm::dmat4 matriks;

		std::map<BlockType, Graphics::OpenGL::ShaderProgram> block_shaders;

		Graphics::OpenGL::ShaderProgram s_lines;

		Graphics::OpenGL::VertexBuffer outline_vbo;
		Graphics::OpenGL::VertexArray outline_vao;

		bool is_fullscreen;
		bool cull_face;
		double fov;

		Graphics::Text gui_text;
		glm::dmat4 gui_projection_matrix;

		void hook_events(EventManager&);
		static GLFWwindow* init_glfw();
		void uninit_glfw();
		void opengl_setup();

		/**
		 * Toggle borderless window mode
		 *
		 * @todo Add real fullscreen mode
		 */
		void toggle_fullscreen();

		void toggle_cull_face();
		void update_projection_matrix();
		void set_camera_view(const glm::dvec3& position, const glm::dvec3& rotation);
		void draw_cube_outline(const Position::BlockInWorld&, const glm::vec4& color);
		Graphics::OpenGL::ShaderProgram& get_block_shader(BlockType);

		#ifdef USE_LIBPNG
		static void write_png_RGB(const char* filename, uint8_t* buf, uint_fast32_t width, uint_fast32_t height, bool reverse_rows = false);
		#endif

		// for GUIs
		Graphics::OpenGL::ShaderProgram s_gui_shape;
		Graphics::OpenGL::VertexBuffer gui_rectangle_vbo;
		Graphics::OpenGL::VertexArray gui_rectangle_vao;
		void draw_rectangle(const glm::dvec2& position, const glm::dvec2& size, const glm::dvec4& color);

	private:
		static GLFWwindow* make_window(bool is_fullscreen);
		static void shim_GL_ARB_direct_state_access();
		static void shim_GL_ARB_separate_shader_objects();
};
