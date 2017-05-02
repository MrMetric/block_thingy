#include "Gfx.hpp"

#include <cerrno>
#include <cstdio>							// C FILE stuff (for libpng use)
#include <cstring>							// strerror
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

#include <easylogging++/easylogging++.hpp>
#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-macros"
#endif
#define PNG_SKIP_SETJMP_CHECK // for libpng < 1.5
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
#include <png.h>

#include "Camera.hpp"
#include "Cube.hpp"
#include "Game.hpp"
#include "Settings.hpp"
#include "block/BlockType.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/primitive.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/char_press.hpp"
#include "util/key_press.hpp"
#include "util/mouse_press.hpp"

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

using std::string;

static window_size_t get_window_size(GLFWwindow* window)
{
	int width;
	int height;
	glfwGetFramebufferSize(window, &width, &height);
	return {width, height};
}

static GLFWwindow* make_window(bool fullscreen);
static void shim_GL_ARB_direct_state_access();
static void shim_GL_ARB_separate_shader_objects();

Gfx* Gfx::instance = nullptr;

Gfx::Gfx()
:
	set_instance(this),
	window(init_glfw()),
	window_size(get_window_size(window)),
	window_mid(glm::dvec2(window_size) / 2.0),
	s_lines("shaders/lines"),
	outline_vbo({3, GL_FLOAT}),
	outline_vao(outline_vbo),
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
	event_manager.add_handler(EventType::change_setting, [this](const Event& event)
	{
		auto e = static_cast<const Event_change_setting&>(event);

		if(e.name == "fullscreen")
		{
			set_fullscreen(*static_cast<const bool*>(e.value));
		}
		else if(e.name == "cull_face")
		{
			set_cull_face(*static_cast<const bool*>(e.value));
		}
		else if(e.name == "screen_shader")
		{
			set_screen_shader(*static_cast<const string*>(e.value));
		}
		else if(e.name == "projection_type"
			 || e.name == "fov"
			 || e.name == "near_plane"
			 || e.name == "far_plane"
			 || e.name == "ortho_size"
		)
		{
			// this is not always necessary, but checking for that is not worth doing
			update_projection_matrix();
		}
		else if(e.name == "min_light")
		{
			const float min_light = static_cast<float>(*static_cast<const double*>(e.value));
			for(auto& p : block_shaders)
			{
				p.second.uniform("min_light", min_light);
			}
		}
	});
}

GLFWwindow* Gfx::init_glfw()
{
	if(!glfwInit())
	{
		throw std::runtime_error("glfwInit() failed");
	}

	GLFWwindow* window = make_window(Settings::get<bool>("fullscreen"));

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		throw std::runtime_error("Error loading GLAD");
	}
	LOG(DEBUG) << "OpenGL " << GLVersion.major << "." << GLVersion.minor << " loaded";
	if(!GLAD_GL_ARB_direct_state_access)
	{
		LOG(WARNING) << "OpenGL extension not found: GL_ARB_direct_state_access";
		shim_GL_ARB_direct_state_access();
	}
	if(!GLAD_GL_ARB_separate_shader_objects)
	{
		LOG(WARNING) << "OpenGL extension not found: GL_ARB_separate_shader_objects";
		shim_GL_ARB_separate_shader_objects();
	}

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height)
	{
		Game::instance->update_framebuffer_size(window_size_t(width, height));
	});
	glfwSetKeyCallback(window, [](GLFWwindow*, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_UNKNOWN)
		{
			return;
		}
		Game::instance->keypress({key, scancode, action, mods});
	});
	glfwSetCharModsCallback(window, [](GLFWwindow*, unsigned int codepoint, int mods)
	{
		Game::instance->charpress({static_cast<char32_t>(codepoint), mods});
	});
	glfwSetMouseButtonCallback(window, [](GLFWwindow*, int button, int action, int mods)
	{
		Game::instance->mousepress({button, action, mods});
	});
	glfwSetCursorPosCallback(window, [](GLFWwindow*, double x, double y)
	{
		Game::instance->mousemove(x, y);
	});

	// TODO: loading screen
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glfwSwapBuffers(window);

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

	if(Settings::get<bool>("cull_face"))
	{
		glEnable(GL_CULL_FACE);
	}
	glCullFace(GL_BACK);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// TODO
	//glEnable(GL_FRAMEBUFFER_SRGB);

	set_screen_shader(Settings::get<string>("screen_shader"));

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

void Gfx::update_framebuffer_size(const window_size_t& window_size)
{
	this->window_size = window_size;
	LOG(DEBUG) << "window size: " << window_size.x << "×" << window_size.y;
	window_mid = glm::dvec2(window_size) / 2.0;
	update_projection_matrix();

	const double width = window_size.x;
	const double height = window_size.y;
	gui_projection_matrix = glm::ortho(0.0, width, height, 0.0, -1.0, 1.0);
	gui_text.set_projection_matrix(gui_projection_matrix);
	s_gui_shape.uniform("matriks", glm::mat4(gui_projection_matrix));

	screen_rt.resize(window_size);
	buf_rt.resize(window_size);
	screen_shader->uniform("tex_size", static_cast<glm::vec2>(window_size));
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

void Gfx::set_fullscreen(const bool fullscreen)
{
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = fullscreen ? mode->height : mode->height * 3 / 4;

	const auto x = (mode->width - width) / 2;
	const auto y = (mode->height - height) / 2;
	glfwSetWindowMonitor(window, fullscreen ? monitor : nullptr, x, y, width, height, mode->refreshRate);
}

void Gfx::set_cull_face(const bool cull_face)
{
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
	const double width = window_size.x;
	const double height = window_size.y;
	const double fov = glm::radians(Settings::get<double>("fov"));
	const double near = Settings::get<double>("near_plane");
	const double far  = Settings::get<double>("far_plane"); // TODO: calculate from chunk render distance
	const string type = Settings::get<string>("projection_type");
	if(type == "ortho")
	{
		const double size = Settings::get<double>("ortho_size");
		projection_matrix = glm::ortho(0.0, size, 0.0, size * height / width, near, far);
	}
	else if(type == "infinite")
	{
		projection_matrix = glm::infinitePerspective(fov, width / height, near);
	}
	else
	{
		if(type != "default")
		{
			LOG(WARNING) << "unknown projection type: " << type;
		}
		projection_matrix = glm::perspectiveFov(fov, width, height, near, far);
	}
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

void Gfx::write_png_RGB(const char* filename, const uint8_t* data, const uint32_t width, const uint32_t height, const bool reverse_rows)
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
	const uint32_t w = width;
	const uint32_t h = height;
	png_set_IHDR(png_ptr, info_ptr, w, h, bit_depth, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	const uint_fast32_t rowsize = png_get_rowbytes(png_ptr, info_ptr);
	if(reverse_rows)
	{
		for(uint_fast32_t y = height; y > 0; --y)
		{
			png_write_row(png_ptr, const_cast<uint8_t*>(data + (y - 1) * rowsize));
		}
	}
	else
	{
		for(uint_fast32_t y = 0; y < height; ++y)
		{
			png_write_row(png_ptr, const_cast<uint8_t*>(data + y * rowsize));
		}
	}
	png_write_end(png_ptr, info_ptr);
	fclose(fp);
	png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, &info_ptr);
}

void Gfx::center_cursor()
{
	glfwSetCursorPos(window, window_mid.x, window_mid.y);
}

static GLFWwindow* make_window(bool fullscreen)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	const int width = fullscreen ? mode->width : mode->width * 3 / 4;
	const int height = fullscreen ? mode->height : mode->height * 3 / 4;
	LOG(DEBUG) << "window size: " << width << "×" << height;
	GLFWwindow* window = glfwCreateWindow(width, height, "Baby's First Voxel Engine", fullscreen ? monitor : nullptr, nullptr);
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

void Gfx::draw_rectangle(glm::dvec2 position, glm::dvec2 size, const glm::dvec4& color)
{
	position = glm::round(position);
	size = glm::round(size);

	const float w = static_cast<float>(size.x);
	const float h = static_cast<float>(size.y);
	const float x = static_cast<float>(position.x);
	const float y = static_cast<float>(position.y);

	float v[] =
	{
		x + w, y + h,
		x + w, y    ,
		x    , y    ,

		x    , y    ,
		x    , y + h,
		x + w, y + h,
	};

	glUseProgram(s_gui_shape.get_name());
	s_gui_shape.uniform("color", glm::vec4(color));

	gui_rectangle_vbo.data(sizeof(v), v, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);
	gui_rectangle_vao.draw(GL_TRIANGLES, 0, sizeof(v) / sizeof(v[0]));
}

void Gfx::draw_border(glm::dvec2 position, glm::dvec2 size, glm::dvec4 border_size, const glm::dvec4& color)
{
	position = glm::round(position);
	size = glm::round(size);

	const float x = static_cast<float>(position.x);
	const float y = static_cast<float>(position.y);
	const float w = static_cast<float>(size.x);
	const float h = static_cast<float>(size.y);
	const float sx1 = static_cast<float>(border_size.x);
	const float sx2 = static_cast<float>(border_size.y);
	const float sy1 = static_cast<float>(border_size.z);
	const float sy2 = static_cast<float>(border_size.w);

	float v[] =
	{
		// left
		x - sx1, y - sy1    ,
		x      , y - sy1    ,
		x      , y + h + sy2,

		x - sx1, y - sy1    ,
		x      , y + h + sy2,
		x - sx1, y + h + sy2,

		// right
		x + w      , y - sy1    ,
		x + w + sx2, y - sy1    ,
		x + w + sx2, y + h + sy2,

		x + w      , y - sy1    ,
		x + w + sx2, y + h + sy2,
		x + w      , y + h + sy2,

		// top
		x    , y - sy1,
		x + w, y - sy1,
		x + w, y      ,

		x    , y - sy1,
		x + w, y      ,
		x    , y      ,

		// bottom
		x    , y + h      ,
		x + w, y + h      ,
		x + w, y + h + sy2,

		x    , y + h      ,
		x + w, y + h + sy2,
		x    , y + h + sy2,
	};

	glUseProgram(s_gui_shape.get_name());
	s_gui_shape.uniform("color", glm::vec4(color));

	gui_rectangle_vbo.data(sizeof(v), v, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);
	gui_rectangle_vao.draw(GL_TRIANGLES, 0, sizeof(v) / sizeof(v[0]));
}

static void shim_GL_ARB_direct_state_access()
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

static void shim_GL_ARB_separate_shader_objects()
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
