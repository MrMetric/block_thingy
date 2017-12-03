#include "Button.hpp"

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Gfx.hpp"
#include "console/Console.hpp"
#include "util/logger.hpp"
#include "util/mouse_press.hpp"

using std::string;

namespace Graphics::GUI::Widget {

Button::Button
(
	const string& text
)
:
	color(0.01, 0.01, 0.02, 0.85),
	hover_color(0, 0, 0, 1),
	mousedown(false)
{
	set_text(text);
	style["size.x"] = 256.0;
	style["size.y"] = 64.0;
}

std::string Button::type() const
{
	return "Button";
}

void Button::draw()
{
	Base::draw();

	Gfx::instance->draw_rectangle(position, size, hover ? hover_color : color);
	Gfx::instance->gui_text.draw(text, text_position);
}

void Button::mousepress(const Util::mouse_press& press)
{
	// TODO: option for left-handed mouse
	if(press.button != GLFW_MOUSE_BUTTON_LEFT)
	{
		return;
	}
	if(!hover)
	{
		if(mousedown && press.action == GLFW_RELEASE)
		{
			// click cancel
			mousedown = false;
		}
	}
	else if(press.action == GLFW_PRESS)
	{
		// mouse down
		mousedown = true;
	}
	else if(mousedown && press.action == GLFW_RELEASE)
	{
		// mouse up (click)
		mousedown = false;
		for(const auto& handler : click_handlers)
		{
			handler();
		}
	}
}

void Button::read_layout(const json& layout)
{
	Base::read_layout(layout);

	set_text(get_layout_var<string>(layout, "text", ""));

	const json command = *layout.find("command");
	if(command.is_string())
	{
		const string c = command;
		on_click([c]()
		{
			Console::instance->run_line(c);
		});
	}
	else if(command.is_array())
	{
		bool good = true;
		for(const json& c : command)
		{
			if(!c.is_string())
			{
				LOG(ERROR) << "Button command list has a non-string (" << c.type_name() << ")\n";
				good = false;
			}
		}
		if(good)
		{
			const std::vector<string> commands = command;
			on_click([commands]()
			{
				for(const string& c : commands)
				{
					Console::instance->run_line(c);
				}
			});
		}
	}
	else
	{
		LOG(ERROR) << "Button command should be a string or a string list, but is " << command.type_name() << '\n';
	}
}

void Button::use_layout()
{
	Base::use_layout();

	text_position = position + (size - text_size) * 0.5;
}

void Button::set_text(const string& text)
{
	if(this->text != text)
	{
		this->text = text;
		text_size = Gfx::instance->gui_text.get_size(text);
	}
}

void Button::on_click(std::function<void()> click_handler)
{
	click_handlers.emplace_back(std::move(click_handler));
}

}
