#include "Gfx.hpp"

#include <cerrno>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>					// glm::mod
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>			// glm::radians
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>		// glm::perspective
#include <glm/gtx/transform.hpp>			// glm::rotate, glm::translate

#include "Camera.hpp"
#include "Game.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "block/Enum/Type.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "event/EventManager.hpp"
#include "event/EventType.hpp"
#include "event/type/Event_change_setting.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/primitive.hpp"
#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"
#include "physics/AABB.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/char_press.hpp"
#include "util/key_press.hpp"
#include "util/logger.hpp"
#include "util/mouse_press.hpp"

#ifdef near
#undef near
#endif

#ifdef far
#undef far
#endif

using std::string;
using Graphics::OpenGL::ShaderProgram;

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
	window(Gfx::init_glfw()),
	window_size(get_window_size(window)),
	window_mid(glm::dvec2(window_size) / 2.0),
	s_lines("shaders/lines"),
	outline_vbo({3, GL_FLOAT}),
	outline_vao(outline_vbo),
	gui_text(Settings::get<string>("font"), static_cast<FT_UInt>(Settings::get<int64_t>("font_size"))),
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
		const auto& e = static_cast<const Event_change_setting&>(event);

		if(e.name == "fullscreen")
		{
			set_fullscreen(*e.new_value.get<bool>());
		}
		else if(e.name == "cull_face")
		{
			set_cull_face(*e.new_value.get<bool>());
		}
		else if(e.name == "screen_shader")
		{
			set_screen_shader(*e.new_value.get<string>());
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
		else if(e.name == "light_smoothing")
		{
			const int light_smoothing = static_cast<int>(*e.new_value.get<int64_t>());
			Game::instance->resource_manager.foreach_ShaderProgram([light_smoothing](Resource<ShaderProgram> r)
			{
				if(Util::string_starts_with(r.get_id(), "shaders/block/"))
				{
					r->uniform("light_smoothing", light_smoothing);
				}
			});
		}
		else if(e.name == "min_light")
		{
			const float min_light = static_cast<float>(*e.new_value.get<double>());
			Game::instance->resource_manager.foreach_ShaderProgram([min_light](Resource<ShaderProgram> r)
			{
				if(Util::string_starts_with(r.get_id(), "shaders/block/"))
				{
					r->uniform("min_light", min_light);
				}
			});
		}
		else if(e.name == "font")
		{
			const string font_path = *e.new_value.get<string>();
			try
			{
				gui_text.set_font(font_path);
			}
			catch(const std::runtime_error& error)
			{
				LOG(ERROR) << error.what() << '\n';
				// TODO: cancel current event
				// triggering an event from an event handler deadlocks
				//Settings::set<string>("font", *e.old_value.get<string>());
			}
		}
		else if(e.name == "font_size")
		{
			const FT_UInt font_size = static_cast<FT_UInt>(*e.new_value.get<int64_t>());
			gui_text.set_font_size(font_size);
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
	LOG(DEBUG) << "OpenGL " << GLVersion.major << '.' << GLVersion.minor << " loaded\n";
	if(!GLAD_GL_ARB_direct_state_access)
	{
		LOG(WARN) << "OpenGL extension not found: GL_ARB_direct_state_access\n";
		shim_GL_ARB_direct_state_access();
	}
	if(!GLAD_GL_ARB_separate_shader_objects)
	{
		LOG(WARN) << "OpenGL extension not found: GL_ARB_separate_shader_objects\n";
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
	LOG(DEBUG) << "window size: " << window_size.x << "×" << window_size.y << '\n';
	window_mid = glm::dvec2(window_size) / 2.0;

	if(window_size.x == 0 || window_size.y == 0)
	{
		// happens when minimizing on Windows
		return;
	}

	update_projection_matrix();

	const double width = window_size.x;
	const double height = window_size.y;
	gui_projection_matrix = glm::ortho(0.0, width, height, 0.0, -1.0, 1.0);
	gui_text.set_projection_matrix(gui_projection_matrix);
	s_gui_shape.uniform("mvp_matrix", glm::mat4(gui_projection_matrix));

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
	projection_matrix = make_projection_matrix(window_size.x, window_size.y);
}

glm::dmat4 Gfx::make_projection_matrix(const double width, const double height)
{
	const double fov = glm::radians(Settings::get<double>("fov"));
	const double near = Settings::get<double>("near_plane");
	const double far  = Settings::get<double>("far_plane"); // TODO: calculate from chunk render distance
	const string type = Settings::get<string>("projection_type");
	if(type == "ortho")
	{
		const double size = Settings::get<double>("ortho_size");
		return glm::ortho(0.0, size, 0.0, size * height / width, near, far);
	}
	if(type == "infinite")
	{
		return glm::infinitePerspective(fov, width / height, near);
	}
	if(type != "default")
	{
		LOG(WARN) << "unknown projection type: " << type << '\n';
	}
	return glm::perspectiveFov(fov, width, height, near, far);
}

glm::dmat4 Gfx::make_rotation_matrix(const glm::dvec3& rotation)
{
	return
	  glm::rotate(glm::radians(rotation.x), glm::dvec3(1, 0, 0))
	* glm::rotate(glm::radians(rotation.y), glm::dvec3(0, 1, 0))
	* glm::rotate(glm::radians(rotation.z), glm::dvec3(0, 0, 1));
}

void Gfx::set_camera_view
(
	const glm::dvec3& position,
	const glm::dmat4& rotation,
	const glm::dmat4& projection_matrix
)
{
	physical_position = position;
	view_matrix_physical = rotation * glm::translate(-1.0 * physical_position);

	graphical_position = glm::mod(position, static_cast<double>(CHUNK_SIZE));
	view_matrix_graphical = rotation * glm::translate(-1.0 * graphical_position);

	vp_matrix = projection_matrix * view_matrix_graphical;
}

void Gfx::draw_box_outline(const glm::dvec3& min_, const glm::dvec3& max_, const glm::dvec4& color)
{
	const glm::dvec3 o = graphical_position - physical_position;
	const glm::vec3 min(min_ + o);
	const glm::vec3 max(max_ + o);

	GLfloat vertexes[] = {
		min.x, min.y, min.z,
		max.x, min.y, min.z,

		min.x, min.y, min.z,
		min.x, max.y, min.z,

		min.x, min.y, min.z,
		min.x, min.y, max.z,

		max.x, max.y, max.z,
		min.x, max.y, max.z,

		max.x, max.y, max.z,
		max.x, min.y, max.z,

		max.x, max.y, max.z,
		max.x, max.y, min.z,

		min.x, min.y, max.z,
		min.x, max.y, max.z,

		min.x, min.y, max.z,
		max.x, min.y, max.z,

		min.x, max.y, min.z,
		max.x, max.y, min.z,

		min.x, max.y, min.z,
		min.x, max.y, max.z,

		max.x, min.y, min.z,
		max.x, max.y, min.z,

		max.x, min.y, min.z,
		max.x, min.y, max.z,
	};
	outline_vbo.data(sizeof(vertexes), vertexes, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);

	s_lines.uniform("mvp_matrix", glm::mat4(vp_matrix));
	s_lines.uniform("color", glm::vec4(color));

	s_lines.use();
	outline_vao.draw(GL_LINES, 0, 12*2);
}

void Gfx::draw_box_outline(const Physics::AABB& aabb, const glm::dvec4& color)
{
	draw_box_outline(aabb.min, aabb.max, color);
}

void Gfx::draw_block_outline(const Position::BlockInWorld& block_pos, const glm::dvec4& color)
{
	glm::dvec3 min(block_pos.x, block_pos.y, block_pos.z);
	draw_box_outline(min, min + 1.0, color);
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
	LOG(DEBUG) << "window size: " << width << "×" << height << '\n';
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
	gui_rectangle_vbo.data(sizeof(v), v, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);

	s_gui_shape.uniform("color", glm::vec4(color));

	s_gui_shape.use();
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
	gui_rectangle_vbo.data(sizeof(v), v, Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw);

	s_gui_shape.uniform("color", glm::vec4(color));

	s_gui_shape.use();
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
