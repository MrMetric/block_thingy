#include "Base.hpp"

#include "Gfx.hpp"
#include "graphics/GUI/Widget/Component/Base.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::gui::widget {

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

void Base::keypress(const util::key_press&)
{
}

void Base::charpress(const util::char_press&)
{
}

void Base::mousepress(const util::mouse_press&)
{
}

void Base::mousemove(const glm::dvec2& mouse_pos)
{
	hover = mouse_pos.x >= position.x && mouse_pos.x < position.x + size.x
		 && mouse_pos.y >= position.y && mouse_pos.y < position.y + size.y;
}

glm::dvec2 Base::get_size() const
{
	glm::dvec2 size;
	const double* d = nullptr; // init to silence false-positive warning
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

void Base::add_modifier(std::shared_ptr<component::Base> m)
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

void Base::read_layout(const json& j)
{
	if(const auto i_id = j.find("id"); i_id != j.cend())
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

	const auto i_layout = j.find("layout");
	if(i_layout == j.cend())
	{
		return;
	}
	const json& layout = *i_layout;

	// TODO: this is shit
	auto translate_vec2 = [this, &layout](const string& name)
	{
		if(const auto i = layout.find(name); i != layout.cend())
		{
			if(i->is_string())
			{
				const string value = i->get<string>();

				string value_x = value;
				util::replace(value_x, "center", "center.x");
				util::replace(value_x, "end"   , "end.x"   );
				util::replace(value_x, "pos"   , "pos.x"   );
				util::replace(value_x, "size"  , "size.x"  );
				style[name + ".x"] = value_x;

				string value_y = value;
				util::replace(value_y, "center", "center.y");
				util::replace(value_y, "end"   , "end.y"   );
				util::replace(value_y, "pos"   , "pos.y"   );
				util::replace(value_y, "size"  , "size.y"  );
				style[name + ".y"] = value_y;
			}
			else if(i->is_number())
			{
				style[name + ".x"] = style[name + ".y"] = i->get<double>();
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
		const json& v = i.value();
		if(v.is_number())
		{
			style[k] = v.get<double>();
		}
		else if(v.is_string())
		{
			if(const string s = v.get<string>(); s != "default")
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
		style_vars["size.x"] >= 0 | rhea::strength::required(),
		style_vars["size.y"] >= 0 | rhea::strength::required(),
		style_vars["pos.x"] == style_vars["center.x"] - style_vars["size.x"] / 2 | rhea::strength::required(),
		style_vars["pos.y"] == style_vars["center.y"] - style_vars["size.y"] / 2 | rhea::strength::required(),
		style_vars["end.x"] == style_vars["pos.x"] + style_vars["size.x"] | rhea::strength::required(),
		style_vars["end.y"] == style_vars["pos.y"] + style_vars["size.y"] | rhea::strength::required(),
	});

	for(const auto& p : style)
	{
		if(const double* d = p.second.get<double>(); d != nullptr)
		{
			solver.add_constraint(style_vars[p.first] == *d);
			continue;
		}

		// TODO: better checking
		if(p.first == "auto_layout")
		{
			continue;
		}
		if(const string* s = p.second.get<string>(); s != nullptr && !s->empty())
		{
			if(util::string_starts_with(*s, "parent."))
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
	size.x = std::max(0.0, style_vars["size.x"].value() - border_size.x - border_size.y);
	size.y = std::max(0.0, style_vars["size.y"].value() - border_size.z - border_size.w);
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
