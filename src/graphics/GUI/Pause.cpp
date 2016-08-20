#include "Pause.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/Widget/Button.hpp"

namespace Graphics {
namespace GUI {

Pause::Pause(Game& game)
	:
	Base(game, WidgetContainerMode::rows)
{
	//root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, -140}, "Resume"     );
	//root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, - 70}, "Save"       );
	//root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0,    0}, "Save & Exit");
	//root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0,   70}, "Exit"       );
	//root.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0,  140}, "Kill Self"  );

	auto& row1 = root.add(WidgetContainerMode::widgets);
	row1.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 0}, "row 1");

	auto& row2 = root.add(WidgetContainerMode::cols);

	auto& col1 = row2.add(WidgetContainerMode::widgets);
	col1.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 0}, "col 1");

	auto& col2 = row2.add(WidgetContainerMode::rows);
	auto& col2_row1 = col2.add(WidgetContainerMode::widgets);
	auto& col2_row2 = col2.add(WidgetContainerMode::widgets);
	col2_row1.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 0}, "col 2 row 1");
	col2_row2.add<Widget::Button>(glm::dvec2{0.5, 0.5}, glm::dvec2{0, 0}, "col 2 row 2");
}

void Pause::init()
{
	glfwSetInputMode(game.gfx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Pause::draw()
{
	parent->draw();
	Base::draw();
}

void Pause::draw_gui()
{
	game.gfx.gui_text.draw("paused", {8, 100});
}

} // namespace GUI
} // namespace Graphics
