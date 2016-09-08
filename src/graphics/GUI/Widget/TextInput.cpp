#include "TextInput.hpp"

#include "Game.hpp"
#include "Gfx.hpp"
#include "graphics/GUI/WidgetContainer.hpp"

using std::string;

namespace Graphics::GUI::Widget {

TextInput::TextInput
(
	WidgetContainer& owner,
	const string& content,
	const string& placeholder
)
:
	Base(owner, {256, 40}),
	content(content),
	placeholder(placeholder),
	hover(false),
	focus(false)
{
}

void TextInput::draw()
{
	Base::draw();

	owner.game.gfx.draw_rectangle(real_position, size, {0.01, 0.01, 0.02, 0.85});

	bool has_content = content.get32().size() > 0;

	glm::dvec2 content_size = has_content ? content.get_size() : glm::dvec2(0, placeholder.get_size().y);
	glm::dvec2 text_pos =
	{
		real_position.x + 8,
		real_position.y + (size.y - content_size.y) * 0.5,
	};

	if(has_content)
	{
		owner.game.gfx.gui_text.draw(content.get32(), glm::round(text_pos));
	}
	else
	{
		owner.game.gfx.gui_text.draw(placeholder.get32(), glm::round(text_pos));
	}

	if(focus)
	{
		glm::dvec2 cursor_pos =
		{
			text_pos.x + content_size.x,
			real_position.y + 2,
		};
		owner.game.gfx.draw_rectangle(cursor_pos, {2, size.y - 4}, {1, 1, 1, 1});
	}
}

void TextInput::keypress(const int key, const int scancode, const int action, const int mods)
{
	if(!focus) return;
	if(action == GLFW_RELEASE) return; // must be press or repeat

	if(key == GLFW_KEY_BACKSPACE)
	{
		content.pop_back();
	}
}

void TextInput::charpress(const char32_t codepoint)
{
	if(!focus) return;

	content += codepoint;
}

void TextInput::mousepress(const int button, const int action, const int mods)
{
	focus = hover;
}

void TextInput::mousemove(const double x, const double y)
{
	hover = x >= real_position.x && x < real_position.x + size.x
		 && y >= real_position.y && y < real_position.y + size.y;
}

}
