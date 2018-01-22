#include "Base.hpp"

#include <cassert>
#include <stack>
#include <utility>

#include "Gfx.hpp"
#include "graphics/GUI/Widget/Container.hpp"
#include "graphics/GUI/Widget/Component/Base.hpp"
#include "util/gui_parser.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::nullopt;
using std::string;

namespace block_thingy::graphics::gui::widget {

Base::Base(Base* const parent)
:
	parent(parent),
	sibling_prev(nullptr),
	sibling_next(nullptr),
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

void Base::keypress(const input::key_press& /*press*/)
{
}

void Base::charpress(const input::char_press& /*press*/)
{
}

void Base::mousepress(const input::mouse_press& /*press*/)
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

void Base::set_border_size(const glm::dvec4& v)
{
	border_size = v;
	// TODO: trigger layout recalculation
}

void Base::set_border_color(const glm::dvec4& v)
{
	border_color = v;
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
			// TODO: add file name to errors
			LOG(ERROR) << type() << " attribute \"id\" must be a string, but is " << i_id->type_name() << '\n';
		}
	}

	const auto i_layout = j.find("layout");
	if(i_layout == j.cend())
	{
		return;
	}
	const json& layout = *i_layout;
	for(const json& expression : layout)
	{
		add_layout_expression(expression);
	}
}

void Base::apply_layout
(
	rhea::simplex_solver& solver,
	Container& root,
	Base::style_vars_t& window_vars
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
			solver.add_constraint(style_vars[p.first] == *d | rhea::strength::weak());
			continue;
		}
	}

	for(const std::vector<string>& expr_parts : layout_expressions)
	{
		std::stack<strict_variant::variant<rhea::linear_expression, double>> stack;
		bool bad = false;
		for(const string& part : expr_parts)
		{
			if(part.empty())
			{
				// note: this will happen only if someone adds bad stuff directly to layout_expressions
				LOG(ERROR) << "expression has an empty token\n";
				bad = true;
				break;
			}
			if(part == "=")
			{
				if(stack.size() != 2)
				{
					if(stack.empty())
					{
						LOG(ERROR) << "operator = is missing its operands\n";
					}
					else if(stack.size() == 1)
					{
						LOG(ERROR) << "operator = is missing an operand\n";
					}
					else
					{
						// this happens if you do stuff like: x + (2 = y)
						// note that util::gui_parser handles this, so it will happen only if a user supplies custom data
						LOG(ERROR) << "operator = has too many operands (" << stack.size() << ")\n";
					}
					bad = true;
				}
				// TODO: check if this is the expression's end
				break;
			}
			if(part == "+" || part == "-" || part == "*" || part == "/")
			{
				// operator
				if(stack.size() < 2)
				{
					if(stack.empty())
					{
						LOG(ERROR) << "operator " + part + " is missing its operands\n";
					}
					else
					{
						LOG(ERROR) << "operator " + part + " is missing an operand\n";
					}
					bad = true;
					break;
				}

				const auto var2 = stack.top(); stack.pop();
				const auto var1 = stack.top(); stack.pop();
				const rhea::linear_expression* rvar1 = var1.get<rhea::linear_expression>();
				const rhea::linear_expression* rvar2 = var2.get<rhea::linear_expression>();
				const double* dvar1 = var1.get<double>();
				const double* dvar2 = var2.get<double>();
				#define DO_OP(OPERATOR) \
				if(part == #OPERATOR) \
				{ \
					if(dvar1 != nullptr) \
					{ \
						if(dvar2 != nullptr) \
						{ \
							stack.emplace(*dvar1 OPERATOR *dvar2); \
						} \
						else \
						{ \
							stack.emplace(*dvar1 OPERATOR *rvar2); \
						} \
					} \
					else \
					{ \
						if(dvar2 != nullptr) \
						{ \
							stack.emplace(*rvar1 OPERATOR *dvar2); \
						} \
						else \
						{ \
							stack.emplace(*rvar1 OPERATOR *rvar2); \
						} \
					} \
				}
				DO_OP(+)
				else DO_OP(-)
				else DO_OP(*)
				else DO_OP(/)
				else
				{
					assert(false);
				}
			}
			else
			{
				// operand
				if(part[0] == '$')
				{
					const std::size_t dot_pos = part.find('.');
					const string that_name = part.substr(1, dot_pos - 1);
					Base* that = root.get_widget_by_id(that_name);
					if(that == nullptr)
					{
						LOG(ERROR) << "element " << that_name << " not found for " << part << '\n';
						bad = true;
						break;
					}
					const string var_name = part.substr(dot_pos + 1);
					auto var = that->get_layout_var(var_name, window_vars);
					if(!var.has_value())
					{
						LOG(ERROR) << "unable to get layout var " << part << '\n';
						bad = true;
						break;
					}
					stack.emplace(*var);
					continue;
				}

				try
				{
					stack.emplace(std::stod(part));
				}
				catch(const std::invalid_argument&)
				{
					auto var = get_layout_var(part, window_vars);
					if(!var.has_value())
					{
						LOG(ERROR) << "unable to get layout var " << part << '\n';
						bad = true;
						break;
					}
					stack.emplace(*var);
				}
				catch(const std::out_of_range&)
				{
					LOG(ERROR) << "number out of range: " << part << '\n';
					bad = true;
					break;
				}
			}
		}

		if(bad)
		{
			continue;
		}

		const auto var1 = stack.top(); stack.pop();
		const auto var2 = stack.top(); stack.pop();
		const rhea::linear_expression* rvar1 = var1.get<rhea::linear_expression>();
		const rhea::linear_expression* rvar2 = var2.get<rhea::linear_expression>();
		const double* dvar1 = var1.get<double>();
		const double* dvar2 = var2.get<double>();
		// TODO: add syntax to specify strength
		if(dvar1 != nullptr)
		{
			if(dvar2 != nullptr)
			{
				// TODO: warning
			}
			else
			{
				solver.add_constraint(*dvar1 == *rvar2 | rhea::strength::strong());
			}
		}
		else
		{
			if(dvar2 != nullptr)
			{
				solver.add_constraint(*rvar1 == *dvar2 | rhea::strength::strong());
			}
			else
			{
				solver.add_constraint(*rvar1 == *rvar2 | rhea::strength::strong());
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

std::optional<rhea::variable> Base::get_layout_var(const string& name, style_vars_t& window_vars)
{
	if(util::string_starts_with(name, "sibling_prev."))
	{
		if(sibling_prev == nullptr)
		{
			return nullopt;
		}
		return sibling_prev->get_layout_var(name.substr(13), window_vars);
	}
	if(util::string_starts_with(name, "sibling_next."))
	{
		if(sibling_next == nullptr)
		{
			return nullopt;
		}
		return sibling_next->get_layout_var(name.substr(13), window_vars);
	}
	if(util::string_starts_with(name, "parent."))
	{
		if(parent == nullptr)
		{
			return window_vars[name.substr(7)];
		}
		return parent->get_layout_var(name.substr(7), window_vars);
	}
	if(util::string_starts_with(name, "window."))
	{
		return window_vars[name.substr(7)];
	}
	return style_vars[name];
}

void Base::add_layout_expression(const string& expression)
{
	add_layout_expression(util::parse_expression(expression));
}

void Base::add_layout_expression(const json& expression)
{
	add_layout_expression(layout_expressions, expression, type());
}

void Base::add_layout_expression(const std::vector<string>& expression)
{
	add_layout_expression(layout_expressions, expression, type());
}

void Base::add_layout_expression
(
	std::vector<std::vector<string>>& expressions,
	const json& jexpression,
	const string& widget_info
)
{
	if(!jexpression.is_array())
	{
		throw std::runtime_error(widget_info + " layout expression must be an array, but is a " + jexpression.type_name());
	}
	std::vector<string> expression;
	expression.reserve(jexpression.size());
	for(const json& jtoken : jexpression)
	{
		if(!jtoken.is_string())
		{
			throw std::runtime_error(widget_info + " layout token must be a string, but is a " + jtoken.type_name());
		}
		expression.emplace_back(jtoken.get<string>());
	}
	add_layout_expression(expressions, expression, widget_info);
}

void Base::add_layout_expression
(
	std::vector<std::vector<string>>& expressions,
	const std::vector<string>&
	expression, const string& widget_info
)
{
	std::vector<string> lx;
	std::vector<string> ly;
	bool has_vec = false;
	for(const string& token : expression)
	{
		if(token.empty())
		{
			throw std::runtime_error(widget_info + " has an empty token");
		}

		string t = token;
		if(const std::size_t dot_pos = t.find_last_of('.'); dot_pos != string::npos)
		{
			t = t.substr(dot_pos + 1);
		}

		if(t == "center" || t == "end" || t == "pos" || t == "size")
		{
			lx.emplace_back(token + ".x");
			ly.emplace_back(token + ".y");
			has_vec = true;
		}
		else
		{
			lx.emplace_back(token);
			ly.emplace_back(token);
		}
	}
	expressions.emplace_back(std::move(lx));
	if(has_vec)
	{
		expressions.emplace_back(std::move(ly));
	}
}

template<typename T>
T Base::get_layout_var
(
	const json& j,
	const string& key,
	const T* default_
) const
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
	if constexpr(std::is_same_v<T, string>)
	{
		if(!i->is_string())
		{
			// TODO: use custom exception class
			throw std::runtime_error("widget property '" + key + "' must be a string, but is " + i->type_name());
		}
	}
	else if constexpr(std::is_same_v<T, double>)
	{
		if(!i->is_number())
		{
			// TODO: use custom exception class
			throw std::runtime_error("widget property '" + key + "' must be a number, but is " + i->type_name());
		}
	}
	else if constexpr(std::is_same_v<T, bool>)
	{
		if(!i->is_boolean())
		{
			// TODO: use custom exception class
			throw std::runtime_error("widget property '" + key + "' must be a bool, but is " + i->type_name());
		}
	}
	return i->get<T>();
}

template<typename T>
T Base::get_layout_var
(
	const json& j,
	const string& key,
	const T& default_
) const
{
	return get_layout_var(j, key, &default_);
}

template string Base::get_layout_var(const json&, const string&, const string*) const;
template string Base::get_layout_var(const json&, const string&, const string&) const;

template double Base::get_layout_var(const json&, const string&, const double*) const;
template double Base::get_layout_var(const json&, const string&, const double&) const;

template bool Base::get_layout_var(const json&, const string&, const bool*) const;
template bool Base::get_layout_var(const json&, const string&, const bool&) const;

}
