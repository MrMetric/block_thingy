#include "text_input.hpp"

#include <algorithm>
#include <cassert>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Gfx.hpp"
#include "settings.hpp"
#include "input/char_press.hpp"
#include "input/key_press.hpp"
#include "input/mouse_press.hpp"
#include "util/clipboard.hpp"
#include "util/misc.hpp"
#include "util/unicode.hpp"

#define M_TAU 6.283185307179586476925286766559005768

using std::string;

namespace block_thingy::graphics::gui::widget {

struct text_input::impl
{
	impl(const string& content, const string& placeholder);

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	component::Text content;
	component::Text placeholder;
	bool enabled;
	bool valid;
	bool focus;
	double blink_start_time;
	std::size_t cursor_pos;
	std::size_t selection_start;
	std::vector<on_change_callback_t> on_change_callbacks;
	std::vector<on_keypress_callback_t> on_keypress_callbacks;
};

text_input::impl::impl(const string& content, const string& placeholder)
:
	content(content),
	placeholder(placeholder),
	valid(true),
	focus(false),
	blink_start_time(0),
	cursor_pos(this->content.get32().size()),
	selection_start(cursor_pos)
{
}

text_input::text_input
(
	Base* const parent,
	const string& content,
	const string& placeholder
)
:
	Base(parent),
	color(0.02, 0.02, 0.04, 0.85),
	color_disabled(0.04, 0.02, 0.02, 0.85),
	color_invalid(0.2, 0.02, 0.04, 0.85),
	pImpl(std::make_unique<impl>(content, placeholder))
{
	style["size.x"] = 256.0;
	style["size.y"] = 40.0;
}

text_input::~text_input()
{
}

string text_input::type() const
{
	return "text_input";
}

void text_input::draw()
{
	Base::draw();

	Gfx::instance->draw_rectangle(position, size, get_color());

	const auto& content = pImpl->content;
	const auto& placeholder = pImpl->placeholder;

	const auto& text = content.empty() ? placeholder : content;
	const glm::dvec2 text_pos
	{
		position.x + 8,
		position.y + (size.y - text.get_size().y) * 0.5,
	};
	text.draw(text_pos, size);

	if(!pImpl->enabled)
	{
		return;
	}

	const auto& cursor_pos = pImpl->cursor_pos;
	const auto& selection_start = pImpl->selection_start;

	const double cursor_offset = content.empty() ? 0 : Gfx::instance->gui_text.get_size(content.get32().substr(0, cursor_pos)).x;
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
	if(pImpl->focus)
	{
		const double time = glfwGetTime() - pImpl->blink_start_time;
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

void text_input::keypress(const input::key_press& press)
{
	if(!pImpl->focus) return;
	if(press.action == GLFW_RELEASE) return; // must be press or repeat

	trigger_on_keypress(press);

	const char* key_name_ = glfwGetKeyName(press.key, press.scancode);
	const string key_name = key_name_ != nullptr ? key_name_ : "";

	auto& content = pImpl->content;
	auto& blink_start_time = pImpl->blink_start_time;
	auto& cursor_pos = pImpl->cursor_pos;
	auto& selection_start = pImpl->selection_start;

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

void text_input::charpress(const input::char_press& press)
{
	if(!pImpl->focus) return;

	auto& content = pImpl->content;
	auto& cursor_pos = pImpl->cursor_pos;
	auto& selection_start = pImpl->selection_start;

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
	pImpl->blink_start_time = glfwGetTime();
}

void text_input::mousepress(const input::mouse_press& press)
{
	// TODO: option for left-handed mouse
	if(press.button != GLFW_MOUSE_BUTTON_LEFT
	|| !pImpl->enabled)
	{
		return;
	}
	focused(hover);
	pImpl->cursor_pos = pImpl->selection_start = pImpl->content.get32().size(); // TODO: put near mouse pointer
}

void text_input::read_layout(const json& layout)
{
	Base::read_layout(layout);

	pImpl->content = get_layout_var<string>(layout, "text", "");
	pImpl->placeholder = get_layout_var<string>(layout, "placeholder", "");
	pImpl->enabled = get_layout_var<bool>(layout, "enabled", true);
	pImpl->valid = get_layout_var<bool>(layout, "valid", true);
}

string text_input::get_text() const
{
	return pImpl->content.get8();
}

void text_input::set_text(const string& text, const bool reset_cursor)
{
	const string old = pImpl->content.get8();
	if(text == old) return;

	pImpl->content = text;

	auto& cursor_pos = pImpl->cursor_pos;
	auto& selection_start = pImpl->selection_start;
	const auto content_size = pImpl->content.get32().size();
	if(reset_cursor)
	{
		cursor_pos = selection_start = content_size;
	}
	else
	{
		cursor_pos = std::min(cursor_pos, content_size);
		selection_start = std::min(selection_start, content_size);
	}

	trigger_on_change(old);
	pImpl->blink_start_time = glfwGetTime();
}

void text_input::clear()
{
	if(pImpl->content.empty())
	{
		return;
	}

	const string old = pImpl->content.get8();

	pImpl->content.clear();
	pImpl->cursor_pos = pImpl->selection_start = 0;

	trigger_on_change(old);
	pImpl->blink_start_time = glfwGetTime();
}

void text_input::delete_selection()
{
	auto& cursor_pos = pImpl->cursor_pos;
	auto& selection_start = pImpl->selection_start;

	if(selection_start == cursor_pos)
	{
		return;
	}

	auto& content = pImpl->content;

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
	pImpl->blink_start_time = glfwGetTime();
}

void text_input::on_change(on_change_callback_t callback)
{
	pImpl->on_change_callbacks.emplace_back(callback);
}

void text_input::trigger_on_change(const string& old_value)
{
	const string new_value = pImpl->content.get8();
	for(on_change_callback_t& callback : pImpl->on_change_callbacks)
	{
		callback(*this, old_value, new_value);
	}
}

void text_input::on_keypress(on_keypress_callback_t callback)
{
	pImpl->on_keypress_callbacks.emplace_back(callback);
}

void text_input::trigger_on_keypress(const input::key_press& press)
{
	for(on_keypress_callback_t& callback : pImpl->on_keypress_callbacks)
	{
		callback(*this, press);
	}
}

const glm::dvec4& text_input::get_color() const
{
	if(!pImpl->enabled)
	{
		return color_disabled;
	}
	if(!pImpl->valid)
	{
		return color_invalid;
	}
	return color;
}

bool text_input::enabled() const
{
	return pImpl->enabled;
}
void text_input::enabled(const bool e)
{
	pImpl->enabled = e;
	if(!e)
	{
		pImpl->focus = false;
	}
}

bool text_input::valid() const
{
	return pImpl->valid;
}
void text_input::valid(const bool v)
{
	pImpl->valid = v;
}

bool text_input::focused() const
{
	return pImpl->focus;
}
void text_input::focused(const bool f)
{
	pImpl->focus = f;
	if(f)
	{
		pImpl->blink_start_time = glfwGetTime();
	}
}

}
