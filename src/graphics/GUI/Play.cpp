#include "Play.hpp"

#include <sstream>

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "Player.hpp"

namespace Graphics {
namespace GUI {

Play::Play(Game& game)
	:
	Base(game),
	s_crosshair("shaders/crosshair"),
	crosshair_vbo({2, GL_FLOAT}),
	crosshair_vao(crosshair_vbo)
{
}

void Play::init()
{
	glClearColor(0.0, 0.0, 0.1, 0.0);
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetCursorPos(game.gfx.window, game.gfx.window_size.x / 2, game.gfx.window_size.y / 2);
}

void Play::close()
{
	game.console.run_line("open_gui pause");
}

void Play::draw()
{
	if(game.gui.get() == this)
	{
		game.step_world();
	}
	game.draw_world();
	Base::draw();
}

void Play::keypress(const int key, const int scancode, const int action, const int mods)
{
	game.keybinder.keypress(key, scancode, action, mods);
}

void Play::mousepress(const int button, const int action, const int mods)
{
	game.keybinder.mousepress(button, action, mods);
}

void Play::mousemove(const double x, const double y)
{
	game.camera.mousemove(x, y);
}

void Play::draw_gui()
{
	draw_crosshair();
	draw_debug_text();
}

void Play::draw_crosshair()
{
	glUseProgram(s_crosshair.get_name());
	crosshair_vao.draw(GL_TRIANGLES, 0, 12);
}

void Play::draw_debug_text()
{
	std::ostringstream ss;
	const auto& pos = game.player.position;
	ss << "x: " << pos.x << "\n";
	ss << "y: " << pos.y << "\n";
	ss << "z: " << pos.z << "\n";
	game.gfx.gui_text.draw(ss.str(), {8.0, 8.0});
}

void Play::update_framebuffer_size(const window_size_t& window_size)
{
	float midX = window_size.x / 2.0f;
	float midY = window_size.y / 2.0f;
	float crosshair_vertex[] = {
		midX - 16, midY - 1,
		midX - 16, midY + 1,
		midX + 16, midY + 1,
		midX - 16, midY - 1,
		midX + 16, midY + 1,
		midX + 16, midY - 1,

		midX - 1, midY - 16,
		midX - 1, midY + 16,
		midX + 1, midY + 16,
		midX - 1, midY - 16,
		midX + 1, midY + 16,
		midX + 1, midY - 16,
	};
	const auto usage_hint = Graphics::OpenGL::VertexBuffer::UsageHint::dynamic_draw;
	crosshair_vbo.data(sizeof(crosshair_vertex), crosshair_vertex, usage_hint);

	s_crosshair.uniform("matriks", glm::mat4(game.gfx.gui_projection_matrix));
}

} // namespace GUI
} // namespace Graphics
