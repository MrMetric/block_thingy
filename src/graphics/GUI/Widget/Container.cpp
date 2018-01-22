#include "Container.hpp"

#include <cassert>
#include <stdexcept>
#include <string>

#include "Gfx.hpp"
#include "settings.hpp"
#include "console/Console.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::gui::widget {

struct Container::impl
{
	impl() = default;

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	std::vector<std::unique_ptr<Base>> widgets;
	std::vector<std::vector<string>> child_layout_expressions;
};

Container::Container(Base* const parent)
:
	Base(parent),
	pImpl(std::make_unique<impl>())
{
}

Container::~Container()
{
}

string Container::type() const
{
	return "container";
}

void Container::draw()
{
	if(settings::get<bool>("show_container_bounds"))
	{
		Gfx::instance->draw_rectangle(position, size, {0.8, 0.4, 0, 0.4});
		Gfx::instance->draw_border(position, size, glm::dvec4(2), {0, 0, 0.4, 0.4});
	}
	for(auto& widget : pImpl->widgets)
	{
		widget->draw();
	}
}

void Container::keypress(const input::key_press& press)
{
	for(auto& widget : pImpl->widgets)
	{
		widget->keypress(press);
	}
}

void Container::charpress(const input::char_press& press)
{
	for(auto& widget : pImpl->widgets)
	{
		widget->charpress(press);
	}
}

void Container::mousepress(const input::mouse_press& press)
{
	for(auto& widget : pImpl->widgets)
	{
		widget->mousepress(press);
	}
}

void Container::mousemove(const glm::dvec2& mouse_pos)
{
	for(auto& widget : pImpl->widgets)
	{
		widget->mousemove(mouse_pos);
	}
}

static double get_auto_layout_spacing(const Base::style_t& style)
{
	const double* d = nullptr; // init to silence false-positive warning
	if(style.count("auto_layout.spacing") != 0
	&& (d = style.at("auto_layout.spacing").get<double>()) != nullptr)
	{
		return *d;
	}
	return 0;
}

static bool get_auto_layout_shrink(const Base::style_t& style)
{
	const double* d = nullptr; // init to silence false-positive warning
	if(style.count("auto_layout.shrink") != 0
	&& (d = style.at("auto_layout.shrink").get<double>()) != nullptr)
	{
		return static_cast<bool>(*d);
	}
	return true;
}

/*
TODO: this does not need to be recalculated every time
note that this->size is set in use_layout
but, the calculated value may need to be available in the parent's apply_layout
*/
glm::dvec2 Container::get_size() const
{
	glm::dvec2 size = Base::get_size();

	const bool shrink = get_auto_layout_shrink(style);
	if(!shrink)
	{
		return size;
	}

	const bool do_x = style.count("size.x") == 0;
	const bool do_y = style.count("size.y") == 0;
	if((do_x || do_y) && style.count("auto_layout") != 0)
	{
		const double spacing = get_auto_layout_spacing(style);

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
		// TODO?: this assumes that nothing has its position set
		for(auto& widget : pImpl->widgets)
		{
			const glm::dvec2 size = widget->get_size();
			if(size[i_x] > total_size[i_x])
			{
				total_size[i_x] = size[i_x];
			}
			total_size[i_y] += size[i_y];
		}
		total_size[i_y] += spacing * (pImpl->widgets.size() - 1);

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

	if(const json::const_iterator i_auto_layout = layout.find("auto_layout");
		i_auto_layout != layout.cend())
	{
		const json& auto_layout = *i_auto_layout;
		if(!auto_layout.is_object())
		{
			throw std::runtime_error(string("auto_layout must be a JSON object, but is a ") + auto_layout.type_name());
		}
		const string type = get_layout_var<string>(auto_layout, "type");
		if(type != "column" && type != "row")
		{
			throw std::runtime_error("auto_layout type must be 'column' or 'row', but is '" + type + '\'');
		}
		style["auto_layout"] = type;
		// TODO: allow setting spacing to a variable
		style["auto_layout.spacing"] = get_layout_var(auto_layout, "spacing", 0.0);
		// this cast is kinda ugly, but it works
		style["auto_layout.shrink"] = static_cast<double>(get_layout_var(auto_layout, "shrink", true));
	}

	if(const json::const_iterator i_child_layout = layout.find("child_layout");
		i_child_layout != layout.cend())
	{
		const json& child_layout = *i_child_layout;
		for(const json& expression : child_layout)
		{
			add_layout_expression(pImpl->child_layout_expressions, expression, this->type() + " child");
		}
	}

	if(const json::const_iterator i_widgets = layout.find("widgets");
		i_widgets != layout.cend())
	{
		const json& widgets = *i_widgets;
		if(!widgets.is_array())
		{
			// TODO: warning/error
		}
		for(const json& w_layout : widgets)
		{
			const string type = get_layout_var<string>(w_layout, "type");
			Base* widget;
			if(type == "button")
			{
				widget = &emplace_back<Button>();
			}
			else if(type == "container")
			{
				widget = &emplace_back<Container>();
			}
			else if(type == "text")
			{
				widget = &emplace_back<Text>();
			}
			else if(type == "text_input")
			{
				widget = &emplace_back<text_input>();
			}
			else
			{
				// TODO: use custom exception class
				throw std::runtime_error("unknown widget type: " + type);
			}

			if(type == "button" || type == "text_input")
			{
				widget->set_border_size(glm::dvec4(2));
				widget->set_border_color(glm::dvec4(1));
			}

			widget->read_layout(w_layout);
		}
	}
}

void Container::apply_layout
(
	rhea::simplex_solver& solver,
	Container& root,
	Base::style_vars_t& window_vars
)
{
	Base::apply_layout(solver, root, window_vars);

	// must be before auto_layout
	for(auto& widget : pImpl->widgets)
	{
		widget->apply_layout(solver, root, window_vars);
	}

	if(style.count("auto_layout") != 0)
	{
		const double spacing = get_auto_layout_spacing(style);

		const string type = *style["auto_layout"].get<string>();
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

		if(get_auto_layout_shrink(style))
		{
			const glm::dvec2 total_size = get_size();
			solver.add_constraints
			({
				style_vars["size.x"] == total_size.x | rhea::strength::weak(),
				style_vars["size.y"] == total_size.y | rhea::strength::weak(),
			});
		}

		rhea::variable* prev_widget_y = nullptr;
		rhea::variable* prev_widget_size_y = nullptr;
		for(auto& widget : pImpl->widgets)
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

	for(const auto& widget : pImpl->widgets)
	{
		widget->use_layout();
	}
}

Base* Container::get_widget_by_id(const string& id)
{
	for(auto& w : pImpl->widgets)
	{
		if(w->id == id)
		{
			return w.get();
		}
		Container* c = dynamic_cast<Container*>(w.get());
		if(c != nullptr)
		{
			Base* w2 = c->get_widget_by_id(id);
			if(w2 != nullptr)
			{
				return w2;
			}
		}
	}
	return nullptr;
}

Base* Container::add_back(std::unique_ptr<Base> w_)
{
	Base* w = w_.get();
	pImpl->widgets.emplace_back(std::move(w_));

	if(pImpl->widgets.size() > 1)
	{
		Base* prev = (pImpl->widgets.end() - 2)->get();
		prev->sibling_next = w;
		w->sibling_prev = prev;
	}

	for(const auto& expr : pImpl->child_layout_expressions)
	{
		w->layout_expressions.emplace_back(expr);
	}

	return w;
}

}
