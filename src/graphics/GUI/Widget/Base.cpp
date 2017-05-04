#include "Base.hpp"

#include "Gfx.hpp"
#include "Util.hpp"
#include "graphics/GUI/Widget/Component/Base.hpp"

using std::string;

namespace Graphics::GUI::Widget {

Base::Base()
:
	hover(false)
{
}

Base::~Base()
{
}

void Base::draw()
{
	Gfx::instance->draw_border(position, size, border_size, border_color);

	for(auto& m : modifiers)
	{
		m->draw(position, size);
	}
}

void Base::keypress(const Util::key_press& press)
{
}

void Base::charpress(const Util::char_press& press)
{
}

void Base::mousepress(const Util::mouse_press& press)
{
}

void Base::mousemove(const double x, const double y)
{
	hover = x >= position.x && x < position.x + size.x
		 && y >= position.y && y < position.y + size.y;
}

glm::dvec2 Base::get_size() const
{
	glm::dvec2 size;
	const double* d;
	if(style.count("size.x") != 0 && (d = style.at("size.x").get<double>()) != nullptr)
	{
		size.x = *d;
	}
	if(style.count("size.y") != 0 && (d = style.at("size.y").get<double>()) != nullptr)
	{
		size.y = *d;
	}
	return size;
}

glm::dvec2 Base::get_position() const
{
	return position;
}

void Base::add_modifier(std::shared_ptr<Component::Base> m)
{
	modifiers.emplace_back(m);
}

void Base::set_border_size(glm::dvec4 v)
{
	border_size = std::move(v);
	// TODO: trigger layout recalculation
}

void Base::set_border_color(glm::dvec4 v)
{
	border_color = std::move(v);
}

void Base::read_layout(const json& layout)
{
	const json::const_iterator i_id = layout.find("id");
	if(i_id != layout.cend())
	{
		if(i_id->is_string())
		{
			id = i_id->get<string>();
		}
		else
		{
			// TODO: warning
		}
	}

	auto translate_vec2 = [this, &layout](const string& name)
	{
		const json::const_iterator i = layout.find(name);
		if(i != layout.cend())
		{
			if(i->is_string())
			{
				const string value = *i;
				// TODO: handle `value == "default"`
				style[name + ".x"] = value + ".x";
				style[name + ".y"] = value + ".y";
			}
			else
			{
				// TODO: warning
			}
		}
	};
	translate_vec2("center");
	translate_vec2("end");
	translate_vec2("pos");
	translate_vec2("size");

	for(json::const_iterator i = layout.cbegin(); i != layout.cend(); ++i)
	{
		const string& k = i.key();
		// TODO: better not-layout checking
		if(k == "type" || k == "id" || k == "text" || k == "command" || k == "placeholder")
		{
			continue;
		}
		const json& j = i.value();
		if(j.is_number())
		{
			style[k] = j.get<double>();
		}
		else if(j.is_string())
		{
			const string s = j.get<string>();
			if(s != "default")
			{
				style[k] = s;
			}
		}
	}
}

void Base::apply_layout
(
	rhea::simplex_solver& solver,
	Base::style_vars_t& parent_vars
)
{
	solver.add_constraints
	({
		style_vars["pos.x"] == style_vars["center.x"] - style_vars["size.x"] / 2,
		style_vars["pos.y"] == style_vars["center.y"] - style_vars["size.y"] / 2,
		style_vars["end.x"] == style_vars["pos.x"] + style_vars["size.x"],
		style_vars["end.y"] == style_vars["pos.y"] + style_vars["size.y"],
	});

	for(const auto& p : style)
	{
		const double* d;
		if((d = p.second.get<double>()) != nullptr)
		{
			solver.add_constraint(style_vars[p.first] == *d);
		}

		// TODO: better checking
		if(p.first == "auto_layout")
		{
			continue;
		}
		const string* s;
		if((s = p.second.get<string>()) != nullptr && !s->empty())
		{
			if(Util::string_starts_with(*s, "parent."))
			{
				solver.add_constraint(style_vars[p.first] == parent_vars[s->substr(7)]);
			}
			else
			{
				solver.add_constraint(style_vars[p.first] == style_vars[*s]);
			}
		}
	}
}

void Base::use_layout()
{
	position.x = style_vars["pos.x"].value() + border_size.x;
	position.y = style_vars["pos.y"].value() + border_size.z;
	size.x = style_vars["size.x"].value() - border_size.x - border_size.y;
	size.y = style_vars["size.y"].value() - border_size.z - border_size.w;
}

template<>
string Base::get_layout_var
(
	const json& j,
	const string& key,
	const string* default_
)
{
	const json::const_iterator i = j.find(key);
	if(i == j.cend())
	{
		if(default_ == nullptr)
		{
			// TODO: use custom exception class
			throw std::runtime_error("required widget property '" + key + "' not found");
		}
		return *default_;
	}
	if(!i->is_string())
	{
		// TODO: use custom exception class
		throw std::runtime_error("widget property '" + key + "' must be a string, but is " + i->type_name());
	}
	return i->get<string>();
}

template<>
string Base::get_layout_var
(
	const json& j,
	const string& key,
	const string& default_
)
{
	return get_layout_var(j, key, &default_);
}

template<>
double Base::get_layout_var
(
	const json& j,
	const string& key,
	const double* default_
)
{
	const json::const_iterator i = j.find(key);
	if(i == j.cend())
	{
		if(default_ == nullptr)
		{
			// TODO: use custom exception class
			throw std::runtime_error("required widget property '" + key + "' not found");
		}
		return *default_;
	}
	if(!i->is_number())
	{
		// TODO: use custom exception class
		throw std::runtime_error("widget property '" + key + "' must be a number, but is " + i->type_name());
	}
	return i->get<double>();
}

template<>
double Base::get_layout_var
(
	const json& j,
	const string& key,
	const double& default_
)
{
	return get_layout_var(j, key, &default_);
}

}
