#include "Container.hpp"

#include <stdexcept>
#include <string>

#include "Gfx.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/TextInput.hpp"

using std::string;

namespace block_thingy::graphics::gui::widget {

Container::Container()
{
}

string Container::type() const
{
	return "Container";
}

void Container::draw()
{
	if(settings::get<bool>("show_container_bounds"))
	{
		Gfx::instance->draw_rectangle(position, size, {0.2, 0.1, 0, 0.4});
		Gfx::instance->draw_border(position, size, glm::dvec4(2), {0, 0, 0.1, 0.4});
	}
	for(auto& widget : widgets)
	{
		widget->draw();
	}
}

void Container::keypress(const util::key_press& press)
{
	for(auto& widget : widgets)
	{
		widget->keypress(press);
	}
}

void Container::charpress(const util::char_press& press)
{
	for(auto& widget : widgets)
	{
		widget->charpress(press);
	}
}

void Container::mousepress(const util::mouse_press& press)
{
	for(auto& widget : widgets)
	{
		widget->mousepress(press);
	}
}

void Container::mousemove(const double x, const double y)
{
	for(auto& widget : widgets)
	{
		widget->mousemove(x, y);
	}
}

static double get_spacing(const Base::style_t& style)
{
	const double* d = nullptr; // init to silence false-positive warning
	if(style.count("auto_layout.spacing") != 0
	&& (d = style.at("auto_layout.spacing").get<double>()) != nullptr)
	{
		return *d;
	}
	return 0;
}

/*
TODO: this does not need to be recalculated every time
note that this->size is set in use_layout
but, the calculated value may need to be available in the parent's apply_layout
*/
glm::dvec2 Container::get_size() const
{
	glm::dvec2 size = Base::get_size();

	const bool do_x = style.count("size.x") == 0;
	const bool do_y = style.count("size.y") == 0;
	if((do_x || do_y) && style.count("auto_layout") != 0)
	{
		const double spacing = get_spacing(style);

		const string type = *style.at("auto_layout").get<string>();
		glm::dvec2 total_size;
		glm::dvec2::length_type i_x, i_y;
		if(type == "column")
		{
			i_x = 0;
			i_y = 1;
		}
		else if(type == "row")
		{
			i_x = 1;
			i_y = 0;
		}
		else
		{
			// TODO: use custom exception class
			throw std::runtime_error("invalid auto_layout type: " + type);
		}
		// TODO: this assumes that nothing has its position set
		for(auto& widget : widgets)
		{
			const glm::dvec2 size = widget->get_size();
			if(size[i_x] > total_size[i_x])
			{
				total_size[i_x] = size[i_x];
			}
			total_size[i_y] += size[i_y];
		}
		total_size[i_y] += spacing * (widgets.size() - 1);

		if(do_x)
		{
			size.x = total_size.x;
		}
		if(do_y)
		{
			size.y = total_size.y;
		}
	}

	return size;
}

void Container::read_layout(const json& layout)
{
	Base::read_layout(layout);

	Base::style_t child_style;
	const json::const_iterator i_auto_layout = layout.find("auto_layout");
	if(i_auto_layout != layout.cend())
	{
		if(!i_auto_layout->is_object())
		{
			// TODO: warning/error
		}
		const json& auto_layout = *i_auto_layout;
		const string type = get_layout_var<string>(auto_layout, "type");
		if(type != "column" && type != "row")
		{
			// TODO: warning/error
		}
		style["auto_layout"] = type;
		// TODO: allow setting spacing to a variable
		style["auto_layout.spacing"] = get_layout_var(auto_layout, "spacing", 0.0);

		for(json::const_iterator i = auto_layout.cbegin(); i != auto_layout.cend(); ++i)
		{
			if(util::string_starts_with(i.key(), "child."))
			{
				const json& j = i.value();
				auto& setting = child_style[i.key().substr(6)];
				if(j.is_string())
				{
					setting = j.get<string>();
				}
				else if(j.is_number())
				{
					setting = j.get<double>();
				}
				else
				{
					// TODO: warning/error
				}
			}
		}
	}

	const json::const_iterator i_widgets = layout.find("widgets");
	if(i_widgets != layout.cend())
	{
		if(!i_widgets->is_array())
		{
			// TODO: warning/error
		}
		for(const json& w_layout : *i_widgets)
		{
			const string type = get_layout_var<string>(w_layout, "type");
			Base* widget;
			if(type == "Button")
			{
				widget = &add<Button>();
			}
			else if(type == "Container")
			{
				widget = &add<Container>();
			}
			else if(type == "Text")
			{
				widget = &add<Text>();
			}
			else if(type == "TextInput")
			{
				widget = &add<TextInput>();
			}
			else
			{
				// TODO: use custom exception class
				throw std::runtime_error("unknown widget type: " + type);
			}

			if(type == "Button" || type == "TextInput")
			{
				widget->set_border_size(glm::dvec4(2));
				widget->set_border_color(glm::dvec4(1));
			}

			widget->read_layout(w_layout);
			for(const auto& p : child_style)
			{
				if(w_layout.count(p.first) == 0)
				{
					widget->style[p.first] = p.second;
				}
			}
		}
	}
}

void Container::apply_layout
(
	rhea::simplex_solver& solver,
	Base::style_vars_t& parent_vars
)
{
	Base::apply_layout(solver, parent_vars);

	// must be before auto_layout
	for(auto& widget : widgets)
	{
		widget->apply_layout(solver, style_vars);
	}

	if(style.count("auto_layout") != 0)
	{
		const double spacing = get_spacing(style);

		const string type = *style["auto_layout"].get<string>();
		const glm::dvec2 total_size = get_size();
		string s_x, s_y;
		if(type == "column")
		{
			s_x = "x";
			s_y = "y";
		}
		else if(type == "row")
		{
			s_x = "y";
			s_y = "x";
		}
		else
		{
			// TODO: use custom exception class
			throw std::runtime_error("invalid auto_layout type: " + type);
		}
		const string pos_x = "pos." + s_x;
		const string pos_y = "pos." + s_y;
		const string center_x = "center." + s_x;
		solver.add_constraints
		({
			style_vars["size.x"] == total_size.x,
			style_vars["size.y"] == total_size.y,
		});

		rhea::variable* prev_widget_y = nullptr;
		rhea::variable* prev_widget_size_y = nullptr;
		for(auto& widget : widgets)
		{
			if(prev_widget_y == nullptr)
			{
				solver.add_constraint(widget->style_vars[pos_y] == style_vars[pos_y]);
			}
			else
			{
				solver.add_constraint(widget->style_vars[pos_y] == *prev_widget_y + *prev_widget_size_y + spacing);
			}

			// if no position is specified, default to the container's center
			solver.add_constraint(widget->style_vars[center_x] == style_vars[center_x] | rhea::strength::weak());

			prev_widget_y = &widget->style_vars[pos_y];
			prev_widget_size_y = &widget->style_vars["size." + s_y];
		}
	}
}

void Container::use_layout()
{
	Base::use_layout();

	for(const auto& widget : widgets)
	{
		widget->use_layout();
	}
}

}
