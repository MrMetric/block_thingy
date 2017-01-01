#pragma once

#include <map>
#include <stdint.h>
#include <string>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "graphics/RenderTarget.hpp"
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
		Gfx();

		Gfx(Gfx&&) = delete;
		Gfx(const Gfx&) = delete;
		void operator=(const Gfx&) = delete;

		GLFWwindow* window;
		window_size_t window_size;
		glm::dvec2 window_mid;

		// I should use less global state here
		glm::dmat4 projection_matrix;
		glm::dmat4 view_matrix_physical;
		glm::dmat4 view_matrix_graphical;
		glm::dvec3 physical_position;
		glm::dvec3 graphical_position;

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
		static void uninit_glfw(GLFWwindow*);
		void opengl_setup();

		Graphics::RenderTarget screen_rt;
		Graphics::RenderTarget buf_rt;
		std::map<std::string, Graphics::OpenGL::ShaderProgram> screen_shaders;
		Graphics::OpenGL::ShaderProgram* screen_shader;
		Graphics::OpenGL::VertexBuffer quad_vbo;
		Graphics::OpenGL::VertexArray quad_vao;
		void set_screen_shader(const std::string&);

		/**
		 * Toggle borderless window mode
		 *
		 * @todo Add real fullscreen mode
		 */
		void toggle_fullscreen();

		void toggle_cull_face();
		void update_projection_matrix();
		void set_camera_view(const glm::dvec3& position, const glm::dvec3& rotation);
		void draw_cube_outline(const Position::BlockInWorld&, const glm::dvec4& color);
		Graphics::OpenGL::ShaderProgram& get_block_shader(BlockType);

		#ifdef USE_LIBPNG
		static void write_png_RGB(const char* filename, uint8_t* buf, uint_fast32_t width, uint_fast32_t height, bool reverse_rows = false);
		#endif

		void center_cursor();

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
