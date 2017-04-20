#include "TextInput.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Gfx.hpp"
#include "util/clipboard.hpp"
#include "util/key_mods.hpp"

using std::string;

namespace Graphics::GUI::Widget {

TextInput::TextInput
(
	const string& content,
	const string& placeholder
)
:
	invalid(false),
	content(content),
	placeholder(placeholder),
	hover(false),
	focus(false)
{
	style["size.x"] = 256.0;
	style["size.y"] = 40.0;
}

std::string TextInput::type() const
{
	return "TextInput";
}

void TextInput::draw()
{
	Base::draw();

	Gfx::instance->draw_rectangle(position, size, invalid ? glm::dvec4(0.2, 0.01, 0.02, 0.85) : glm::dvec4(0.01, 0.01, 0.02, 0.85));

	bool has_content = content.get32().size() > 0;

	glm::dvec2 content_size = has_content ? content.get_size() : glm::dvec2(0, placeholder.get_size().y);
	glm::dvec2 text_pos =
	{
		position.x + 8,
		position.y + (size.y - content_size.y) * 0.5,
	};

	if(has_content)
	{
		Gfx::instance->gui_text.draw(content.get32(), glm::round(text_pos));
	}
	else
	{
		Gfx::instance->gui_text.draw(placeholder.get32(), glm::round(text_pos), glm::dvec3(0.6));
	}

	if(focus)
	{
		glm::dvec2 cursor_pos =
		{
			text_pos.x + content_size.x,
			position.y + 2,
		};
		Gfx::instance->draw_rectangle(cursor_pos, {2, size.y - 4}, {1, 1, 1, 1});
	}
}

void TextInput::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
	if(!focus) return;
	if(action == GLFW_RELEASE) return; // must be press or repeat

	const char* key_name_ = glfwGetKeyName(key, scancode);
	const string key_name = key_name_ != nullptr ? key_name_ : "";

	if(mods.is(false, true, false, false) && key_name == "v")
	{
		content += Util::Clipboard::get_text();
		trigger_on_change();
		return;
	}

	if(key == GLFW_KEY_BACKSPACE)
	{
		if(mods.none() || mods.is(true, false, false, false))
		{
			if(content.str_size() > 0)
			{
				content.pop_back();
				trigger_on_change();
			}
		}
		else if(mods.is(false, true, false, false))
		{
			// TODO
		}
	}
}

void TextInput::charpress(const char32_t codepoint, const Util::key_mods mods)
{
	if(!focus) return;

	content += codepoint;
	trigger_on_change();
}

void TextInput::mousepress(const int button, const int action, const Util::key_mods)
{
	// TODO: option for left-handed mouse
	if(button == GLFW_MOUSE_BUTTON_LEFT)
	{
		focus = hover;
	}
}

void TextInput::mousemove(const double x, const double y)
{
	hover = x >= position.x && x < position.x + size.x
		 && y >= position.y && y < position.y + size.y;
}

void TextInput::read_layout(const json& layout)
{
	Base::read_layout(layout);

	content = get_layout_var<string>(layout, "text", "");
	placeholder = get_layout_var<string>(layout, "placeholder", "");
}

void TextInput::set_text(const string& text)
{
	content = text;
}

void TextInput::on_change(on_change_callback_t callback)
{
	on_change_callbacks.emplace_back(callback);
}

void TextInput::trigger_on_change()
{
	string s = content.get8();
	for(on_change_callback_t& callback : on_change_callbacks)
	{
		callback(*this, s);
	}
}

}
