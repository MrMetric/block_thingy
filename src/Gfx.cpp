#include "Gfx.hpp"

#include <cerrno>
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
#include "util/key_mods.hpp"

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

using std::cout;
using std::string;

static window_size_t get_window_size(GLFWwindow* window)
{
	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);
	return {width, height};
}

Gfx::Gfx()
:
	window(init_glfw()),
	window_size(get_window_size(window)),
	window_mid(glm::dvec2(window_size) / 2.0),
	s_lines("shaders/lines"),
	outline_vbo({3, GL_FLOAT}),
	outline_vao(outline_vbo),
	is_fullscreen(false),
	fov(75),
	gui_text("fonts/Anonymous Pro/Anonymous Pro.ttf", 24),
	screen_rt(window_size, 8),
	buf_rt(window_size),
	quad_vbo({3, GL_BYTE}),
	quad_vao(quad_vbo),
	s_gui_shape("shaders/gui_shape"),
	gui_rectangle_vbo({2, GL_FLOAT}),
	gui_rectangle_vao(gui_rectangle_vbo)
{
	opengl_setup();
}

void Gfx::hook_events(EventManager& event_manager)
{
	event_manager.add_handler(EventType::window_size_change, [this](const Event& event)
	{
		auto e = static_cast<const Event_window_size_change&>(event);

		window_size = e.window_size;
		cout << "window size: " << window_size.x << "×" << window_size.y << "\n";
		window_mid = glm::dvec2(window_size) / 2.0;
		update_projection_matrix();

		const double width = e.window_size.x;
		const double height = e.window_size.y;
		gui_projection_matrix = glm::ortho(0.0, width, height, 0.0, -1.0, 1.0);
		gui_text.set_projection_matrix(gui_projection_matrix);
		s_gui_shape.uniform("matriks", glm::mat4(gui_projection_matrix));

		screen_rt.resize(window_size);
		buf_rt.resize(window_size);
		screen_shader->uniform("tex_size", static_cast<glm::vec2>(window_size));
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
		throw std::runtime_error("Error loading GLAD");
	}
	cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << " loaded\n";
	if(!GLAD_GL_ARB_direct_state_access)
	{
		cout << "OpenGL extension not found: GL_ARB_direct_state_access\n";
		shim_GL_ARB_direct_state_access();
	}
	if(!GLAD_GL_ARB_separate_shader_objects)
	{
		cout << "OpenGL extension not found: GL_ARB_separate_shader_objects\n";
		shim_GL_ARB_separate_shader_objects();
	}

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
	{
		Game::instance->update_framebuffer_size(window_size_t(width, height));
	});
	glfwSetKeyCallback(window, [](GLFWwindow*, int key, int scancode, int action, int mods)
	{
		Game::instance->keypress(key, scancode, action, Util::key_mods(mods));
	});
	glfwSetCharModsCallback(window, [](GLFWwindow*, unsigned int codepoint, int mods)
	{
		Game::instance->charpress(static_cast<char32_t>(codepoint), Util::key_mods(mods));
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int mods)
	{
		Game::instance->mousepress(button, action, Util::key_mods(mods));
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y)
	{
		Game::instance->mousemove(x, y);
	});

	return window;
}

void Gfx::uninit_glfw(GLFWwindow* window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Gfx::opengl_setup()
{
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	cull_face = true;
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	set_screen_shader("default");

	const GLbyte quad_vertex_buffer_data[] =
	{
		-1, -1, 0,
		 1, -1, 0,
		-1,  1, 0,
		-1,  1, 0,
		 1, -1, 0,
		 1,  1, 0,
	};
	quad_vbo.data(sizeof(quad_vertex_buffer_data), quad_vertex_buffer_data, Graphics::OpenGL::VertexBuffer::UsageHint::static_draw);
}

void Gfx::set_screen_shader(const string& name)
{
	const auto i = screen_shaders.find(name);
	if(i != screen_shaders.cend())
	{
		screen_shader = &i->second;
	}
	else
	{
		const string path = "shaders/screen/" + name;
		screen_shader = &screen_shaders.emplace(name, path).first->second;

		// the default value is 0, so setting it explicitly is not needed
		//screen_shader->uniform("tex", 0);
	}
	screen_shader->uniform("tex_size", static_cast<glm::vec2>(window_size));
}

void Gfx::toggle_fullscreen()
{
	is_fullscreen = !is_fullscreen;
	cout << "fullscreen = " << is_fullscreen << "\n";

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = is_fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = is_fullscreen ? mode->height : mode->height * 3 / 4;

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

	physical_position = position;
	graphical_position = glm::mod(position, static_cast<double>(CHUNK_SIZE));
	view_matrix_physical = view_matrix * glm::translate(-1.0 * physical_position);
	view_matrix_graphical = view_matrix * glm::translate(-1.0 * graphical_position);

	matriks = projection_matrix * view_matrix_graphical;
}

// TODO: use GL_LINES
void Gfx::draw_cube_outline(const Position::BlockInWorld& block_pos, const glm::dvec4& color)
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
	s_lines.uniform("matriks", glm::mat4(matriks));
	s_lines.uniform("color", glm::vec4(color));

	outline_vao.draw(GL_LINE_STRIP, 0, 16);
}

Graphics::OpenGL::ShaderProgram& Gfx::get_block_shader(const BlockType type)
{
	const auto i = block_shaders.find(type);
	if(i == block_shaders.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<block_type_id_t>(type)));
	}
	return i->second;
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

void Gfx::center_cursor()
{
	glfwSetCursorPos(window, window_mid.x, window_mid.y);
}

GLFWwindow* Gfx::make_window(bool is_fullscreen)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = is_fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = is_fullscreen ? mode->height : mode->height * 3 / 4;
	cout << "window size: " << width << "×" << height << "\n";
	GLFWwindow* window = glfwCreateWindow(width, height, "Baby's First Voxel Engine", nullptr, nullptr);
	if(window == nullptr)
	{
		glfwTerminate();
		throw std::runtime_error("error creating window");
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync

	glfwSetWindowPos(window, (mode->width - width) / 2, (mode->height - height) / 2);

	return window;
}

void Gfx::draw_rectangle(const glm::dvec2& position, const glm::dvec2& size, const glm::dvec4& color)
{
	const float w = static_cast<float>(size.x);
	const float h = static_cast<float>(size.y);
	const float x = static_cast<float>(position.x);
	const float y = static_cast<float>(position.y);

	float v[] =
	{
		x    , y    ,
		x + w, y    ,
		x + w, y + h,

		x    , y    ,
		x    , y + h,
		x + w, y + h,
	};

	glUseProgram(s_gui_shape.get_name());
	s_gui_shape.uniform("color", glm::vec4(color));

	gui_rectangle_vbo.data(sizeof(v), v, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);
	gui_rectangle_vao.draw(GL_TRIANGLES, 0, 6);
}

void Gfx::shim_GL_ARB_direct_state_access()
{
	glCreateBuffers = [](GLsizei n, GLuint* ids) -> void
	{
		glGenBuffers(n, ids);
	};
	// TODO: handle target != GL_ARRAY_BUFFER
	glNamedBufferData = [](GLuint buffer, GLsizeiptr size, const void* data, GLenum usage) -> void
	{
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, size, data, usage);
	};

	glCreateVertexArrays = [](GLsizei n, GLuint* ids) -> void
	{
		glGenVertexArrays(n, ids);
	};
	glEnableVertexArrayAttrib = [](GLuint vaobj, GLuint index) -> void
	{
		glBindVertexArray(vaobj);
		glEnableVertexAttribArray(index);
	};
	glDisableVertexArrayAttrib = [](GLuint vaobj, GLuint index) -> void
	{
		glBindVertexArray(vaobj);
		glDisableVertexAttribArray(index);
	};

	glCreateTextures = [](GLenum target, GLsizei n, GLuint* ids) -> void
	{
		glGenTextures(n, ids);
	};
	glTextureParameteri = nullptr;

	glCreateFramebuffers = [](GLsizei n, GLuint* ids) -> void
	{
		glGenFramebuffers(n, ids);
	};
	glCheckNamedFramebufferStatus = [](GLuint framebuffer, GLenum target) -> GLenum
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		return glCheckFramebufferStatus(target);
	};
	glNamedFramebufferRenderbuffer = [](GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) -> void
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, attachment, renderbuffertarget, renderbuffer);
	};
	glNamedFramebufferTexture = [](GLuint framebuffer, GLenum attachment, GLuint texture, GLint level) -> void
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glFramebufferTexture(GL_READ_FRAMEBUFFER, attachment, texture, level);
	};

	glCreateRenderbuffers = [](GLsizei n, GLuint* ids) -> void
	{
		glGenRenderbuffers(n, ids);
	};
	glNamedRenderbufferStorage = [](GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height) -> void
	{
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
	};
	glNamedRenderbufferStorageMultisample = [](GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) -> void
	{
		glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
	};
}

void Gfx::shim_GL_ARB_separate_shader_objects()
{
	#define UNIFORM(name, type) \
	glProgramUniform1##name = [](GLuint program, GLint location, const type x) -> void \
	{ \
		glUseProgram(program); \
		glUniform1##name(location, x); \
	}; \
	glProgramUniform2##name = [](GLuint program, GLint location, const type x, const type y) -> void \
	{ \
		glUseProgram(program); \
		glUniform2##name(location, x, y); \
	}; \
	glProgramUniform3##name = [](GLuint program, GLint location, const type x, const type y, const type z) -> void \
	{ \
		glUseProgram(program); \
		glUniform3##name(location, x, y, z); \
	}; \
	glProgramUniform4##name = [](GLuint program, GLint location, const type x, const type y, const type z, const type w) -> void \
	{ \
		glUseProgram(program); \
		glUniform4##name(location, x, y, z, w); \
	}
	UNIFORM(i, GLint);
	UNIFORM(ui, GLuint);
	UNIFORM(f, GLfloat);
	#ifdef GL_ARB_gpu_shader_fp64
	UNIFORM(d, GLdouble);
	#endif

	#define UVEC(name, type) glProgramUniform##name##v = [](GLuint program, GLint location, GLsizei count, const type* value) -> void \
	{ \
		glUseProgram(program); \
		glUniform##name##v(location, count, value); \
	}
	UVEC(2i, GLint);
	UVEC(3i, GLint);
	UVEC(3i, GLint);
	UVEC(4i, GLint);
	UVEC(2ui, GLuint);
	UVEC(3ui, GLuint);
	UVEC(3ui, GLuint);
	UVEC(4ui, GLuint);
	UVEC(2f, GLfloat);
	UVEC(3f, GLfloat);
	UVEC(3f, GLfloat);
	UVEC(4f, GLfloat);

	#define UMATRIX(name, type) glProgramUniformMatrix##name##v = [](GLuint program, GLint location, GLsizei count, GLboolean transpose, const type* value) -> void \
	{ \
		glUseProgram(program); \
		glUniformMatrix##name##v(location, count, transpose, value); \
	}
	UMATRIX(2f, GLfloat);
	UMATRIX(3f, GLfloat);
	UMATRIX(4f, GLfloat);
	UMATRIX(2x3f, GLfloat);
	UMATRIX(3x2f, GLfloat);
	UMATRIX(2x4f, GLfloat);
	UMATRIX(4x2f, GLfloat);
	UMATRIX(3x4f, GLfloat);
	UMATRIX(4x3f, GLfloat);
	#ifdef GL_ARB_gpu_shader_fp64
	UMATRIX(2d, GLdouble);
	UMATRIX(3d, GLdouble);
	UMATRIX(4d, GLdouble);
	UMATRIX(2x3d, GLdouble);
	UMATRIX(3x2d, GLdouble);
	UMATRIX(2x4d, GLdouble);
	UMATRIX(4x2d, GLdouble);
	UMATRIX(3x4d, GLdouble);
	UMATRIX(4x3d, GLdouble);
	#endif
}
