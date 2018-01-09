#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <json.hpp>
#include <rhea/simplex_solver.hpp>
#include <rhea/variable.hpp>
#include <strict_variant/variant.hpp>

#include "fwd/graphics/GUI/Widget/Container.hpp"
#include "fwd/graphics/GUI/Widget/Component/Base.hpp"
#include "fwd/util/char_press.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"

namespace block_thingy::graphics::gui::widget {

class Base
{
public:
	Base(Base* parent);
	virtual ~Base();

	Base(Base&&) = delete;
	Base(const Base&) = delete;
	Base& operator=(Base&&) = delete;
	Base& operator=(const Base&) = delete;

	virtual std::string type() const = 0;

	virtual void draw();

	virtual void keypress(const util::key_press&);
	virtual void charpress(const util::char_press&);
	virtual void mousepress(const util::mouse_press&);
	virtual void mousemove(const glm::dvec2& position);

	virtual glm::dvec2 get_size() const;
	glm::dvec2 get_position() const;

	void add_modifier(std::shared_ptr<component::Base>);

	void set_border_size(const glm::dvec4&);
	void set_border_color(const glm::dvec4&);

	using style_t = std::map<std::string, strict_variant::variant<std::string, double>>;
	using style_vars_t = std::map<std::string, rhea::variable>;

	virtual void read_layout(const json&);
	virtual void apply_layout(rhea::simplex_solver&, Container& root, style_vars_t& window_vars);
	virtual void use_layout();
	rhea::variable& get_layout_var(const std::string& name, style_vars_t& window_vars);

	Base* parent;
	style_t style;
	std::vector<std::vector<std::string>> layout_expressions;
	style_vars_t style_vars;
	std::string id;

protected:
	glm::dvec2 size;
	glm::dvec2 position;

	glm::dvec4 border_size;
	glm::dvec4 border_color;

	bool hover;

	template<typename T>
	T get_layout_var
	(
		const json& layout,
		const std::string& key,
		const T* default_ = nullptr
	);
	template<typename T>
	T get_layout_var
	(
		const json& layout,
		const std::string& key,
		const T& default_
	);

private:
	std::vector<std::shared_ptr<component::Base>> modifiers;
};

template<> char Base::get_layout_var(const json&, const std::string&, const char*) = delete;
template<> char Base::get_layout_var(const json&, const std::string&, const char&) = delete;

}
