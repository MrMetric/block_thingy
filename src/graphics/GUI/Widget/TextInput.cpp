#include "TextInput.hpp"

#include <algorithm>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Gfx.hpp"
#include "settings.hpp"
#include "util/char_press.hpp"
#include "util/clipboard.hpp"
#include "util/key_press.hpp"
#include "util/misc.hpp"
#include "util/mouse_press.hpp"
#include "util/unicode.hpp"

#define M_TAU 6.283185307179586476925286766559005768

using std::string;

namespace block_thingy::graphics::gui::widget {

TextInput::TextInput
(
	const string& content,
	const string& placeholder
)
:
	invalid(false),
	content(content),
	placeholder(placeholder),
	focus(false),
	blink_start_time(0),
	cursor_pos(this->content.get32().size()),
	selection_start(cursor_pos)
{
	style["size.x"] = 256.0;
	style["size.y"] = 40.0;
}

string TextInput::type() const
{
	return "TextInput";
}

void TextInput::draw()
{
	Base::draw();

	Gfx::instance->draw_rectangle(position, size, invalid ? glm::dvec4(0.2, 0.01, 0.02, 0.85) : glm::dvec4(0.01, 0.01, 0.02, 0.85));

	glm::dvec2 text_pos;
	double cursor_offset;
	if(!content.empty())
	{
		content.draw(position, size);
		text_pos = content.draw_position(position, size);
		cursor_offset = Gfx::instance->gui_text.get_size(content.get32().substr(0, cursor_pos)).x;
	}
	else
	{
		placeholder.draw(position, size);
		text_pos = placeholder.draw_position(position, size);
		cursor_offset = 0;
	}

	if(selection_start != cursor_pos)
	{
		assert(!content.empty());
		const std::u32string t = content.get32();
		std::size_t pos1, pos2;
		double start_offset;
		if(selection_start < cursor_pos)
		{
			pos1 = selection_start;
			pos2 = cursor_pos;
			start_offset = Gfx::instance->gui_text.get_size(t.substr(0, pos1)).x;
		}
		else
		{
			pos1 = cursor_pos;
			pos2 = selection_start;
			start_offset = cursor_offset;
		}
		const glm::dvec2 selection_pos
		{
			text_pos.x + start_offset,
			position.y + 2,
		};
		const glm::dvec2 selection_size
		{
			Gfx::instance->gui_text.get_size(t.substr(pos1, pos2 - pos1)).x,
			size.y - 2*2,
		};
		Gfx::instance->draw_rectangle(selection_pos, selection_size, {1, 1, 1, 0.4});
	}
	if(focus)
	{
		const double time = glfwGetTime() - blink_start_time;
		const double blink_rate = settings::get<double>("cursor_blink_rate");
		double alpha;
		if(blink_rate == 0.5)
		{
			// the constant is acos(0.25)
			alpha = std::clamp(2*std::cos(M_TAU * time - 1.318116071652818) + 0.5, 0.0, 1.0);
		}
		else
		{
			alpha = 1 - static_cast<uint_fast32_t>(time / blink_rate) % 2;
		}
		const glm::dvec2 cursor_pos
		{
			text_pos.x + cursor_offset - 2/2,
			position.y + 2,
		};
		Gfx::instance->draw_rectangle(cursor_pos, {2, size.y - 2*2}, {1, 1, 1, alpha});
	}
}

void TextInput::keypress(const util::key_press& press)
{
	if(!focus) return;
	if(press.action == GLFW_RELEASE) return; // must be press or repeat

	trigger_on_keypress(press);

	const char* key_name_ = glfwGetKeyName(press.key, press.scancode);
	const string key_name = key_name_ != nullptr ? key_name_ : "";

	if(press.mods.ctrl_only())
	{
		if(key_name == "c")
		{
			const std::u32string t = content.get32();
			if(selection_start < cursor_pos)
			{
				util::clipboard::set_text(t.substr(selection_start, cursor_pos - selection_start));
			}
			else if(selection_start > cursor_pos)
			{
				util::clipboard::set_text(t.substr(cursor_pos, selection_start - cursor_pos));
			}
			else
			{
				util::clipboard::set_text(t);
			}
			return;
		}
		if(key_name == "v") // paste
		{
			std::u32string ct = util::utf8_to_utf32(util::clipboard::get_text());
			util::delete_element(ct, '\n');
			util::delete_element(ct, '\r');
			if(!ct.empty())
			{
				const string old = content.get8();
				const std::u32string t = content.get32();

				if(selection_start > cursor_pos)
				{
					std::swap(selection_start, cursor_pos);
				}
				content = t.substr(0, selection_start) + ct + t.substr(cursor_pos);
				selection_start += ct.size();
				cursor_pos = selection_start;

				trigger_on_change(old);
				blink_start_time = glfwGetTime();
			}
			return;
		}
		if(key_name == "a") // select all
		{
			const std::size_t len = content.get32().size();
			if(selection_start != 0 || cursor_pos != len)
			{
				selection_start = 0;
				cursor_pos = len;
				blink_start_time = glfwGetTime();
			}
			return;
		}
	}

	if(press.key == GLFW_KEY_BACKSPACE)
	{
		if(press.mods.none() || press.mods.shift_only())
		{
			if(selection_start != cursor_pos)
			{
				delete_selection();
			}
			else if(cursor_pos > 0)
			{
				const string old = content.get8();
				const std::u32string t = content.get32();

				content = t.substr(0, cursor_pos - 1) + t.substr(cursor_pos);
				cursor_pos -= 1;
				selection_start -= 1;

				trigger_on_change(old);
				blink_start_time = glfwGetTime();
			}
		}
		else if(press.mods.is(true, true, false, false)) // Shift+Ctrl
		{
			if(cursor_pos > 0)
			{
				const string old = content.get8();
				const std::u32string t = content.get32();

				content = t.substr(std::max(cursor_pos, selection_start));
				cursor_pos = selection_start = 0;

				trigger_on_change(old);
				blink_start_time = glfwGetTime();
			}
		}
	}
	else if(press.key == GLFW_KEY_DELETE)
	{
		if(press.mods.none() || press.mods.shift_only())
		{
			if(selection_start != cursor_pos)
			{
				delete_selection();
			}
			else if(cursor_pos < content.get32().size())
			{
				const string old = content.get8();
				const std::u32string t = content.get32();

				content = t.substr(0, cursor_pos) + t.substr(cursor_pos + 1);

				trigger_on_change(old);
				blink_start_time = glfwGetTime();
			}
		}
	}
	else if(press.key == GLFW_KEY_LEFT)
	{
		if(selection_start != cursor_pos && !press.mods.shift())
		{
			if(selection_start > cursor_pos)
			{
				selection_start = cursor_pos;
			}
			else
			{
				cursor_pos = selection_start;
			}
			blink_start_time = glfwGetTime();
		}
		else if(cursor_pos > 0)
		{
			cursor_pos -= 1;
			if(!press.mods.shift())
			{
				selection_start = cursor_pos;
			}
			blink_start_time = glfwGetTime();
		}
	}
	else if(press.key == GLFW_KEY_RIGHT)
	{
		if(selection_start != cursor_pos && !press.mods.shift())
		{
			if(selection_start < cursor_pos)
			{
				selection_start = cursor_pos;
			}
			else
			{
				cursor_pos = selection_start;
			}
			blink_start_time = glfwGetTime();
		}
		else if(cursor_pos < content.get32().size())
		{
			cursor_pos += 1;
			if(!press.mods.shift())
			{
				selection_start = cursor_pos;
			}
			blink_start_time = glfwGetTime();
		}
	}
	else if(press.key == GLFW_KEY_HOME)
	{
		if(cursor_pos != 0)
		{
			cursor_pos = 0;
			if(!press.mods.shift())
			{
				selection_start = 0;
			}
			blink_start_time = glfwGetTime();
		}
		else if(selection_start != 0 && !press.mods.shift())
		{
			selection_start = 0;
			blink_start_time = glfwGetTime();
		}
	}
	else if(press.key == GLFW_KEY_END)
	{
		const auto end = content.get32().size();
		if(cursor_pos != end)
		{
			cursor_pos = end;
			if(!press.mods.shift())
			{
				selection_start = cursor_pos;
			}
			blink_start_time = glfwGetTime();
		}
		else if(selection_start != end)
		{
			selection_start = end;
			blink_start_time = glfwGetTime();
		}
	}
}

void TextInput::charpress(const util::char_press& press)
{
	if(!focus) return;

	const string old = content.get8();
	const std::u32string t = content.get32();

	if(selection_start > cursor_pos)
	{
		std::swap(selection_start, cursor_pos);
	}
	content = t.substr(0, selection_start) + press.codepoint + t.substr(cursor_pos);
	selection_start += 1;
	cursor_pos = selection_start;

	trigger_on_change(old);
	blink_start_time = glfwGetTime();
}

void TextInput::mousepress(const util::mouse_press& press)
{
	// TODO: option for left-handed mouse
	if(press.button == GLFW_MOUSE_BUTTON_LEFT)
	{
		set_focus(hover);
		cursor_pos = selection_start = content.get32().size(); // TODO: put near mouse pointer
	}
}

void TextInput::read_layout(const json& layout)
{
	Base::read_layout(layout);

	content = get_layout_var<string>(layout, "text", "");
	placeholder = get_layout_var<string>(layout, "placeholder", "");
}

string TextInput::get_text() const
{
	return content.get8();
}

void TextInput::set_text(const string& text)
{
	const string old = content.get8();
	if(text == old) return;

	content = text;
	cursor_pos = selection_start = content.get32().size();

	trigger_on_change(old);
	blink_start_time = glfwGetTime();
}

void TextInput::clear()
{
	if(content.empty()) return;

	const string old = content.get8();

	content.clear();
	cursor_pos = selection_start = 0;

	trigger_on_change(old);
	blink_start_time = glfwGetTime();
}

// TODO: using this will not unfocus any other focused text input
void TextInput::set_focus(const bool focus)
{
	this->focus = focus;
	if(focus)
	{
		blink_start_time = glfwGetTime();
	}
}

void TextInput::on_change(on_change_callback_t callback)
{
	on_change_callbacks.emplace_back(callback);
}

void TextInput::trigger_on_change(const string& old_value)
{
	const string new_value = content.get8();
	for(on_change_callback_t& callback : on_change_callbacks)
	{
		callback(*this, old_value, new_value);
	}
}

void TextInput::on_keypress(on_keypress_callback_t callback)
{
	on_keypress_callbacks.emplace_back(callback);
}

void TextInput::trigger_on_keypress(const util::key_press& press)
{
	for(on_keypress_callback_t& callback : on_keypress_callbacks)
	{
		callback(*this, press);
	}
}

void TextInput::delete_selection()
{
	assert(selection_start != cursor_pos);
	const string old = content.get8();
	const std::u32string t = content.get32();

	if(selection_start < cursor_pos)
	{
		content = t.substr(0, selection_start) + t.substr(cursor_pos);
		cursor_pos = selection_start;
	}
	else
	{
		content = t.substr(0, cursor_pos) + t.substr(selection_start);
		selection_start = cursor_pos;
	}

	trigger_on_change(old);
	blink_start_time = glfwGetTime();
}

}
