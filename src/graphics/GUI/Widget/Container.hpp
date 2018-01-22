#pragma once
#include "Base.hpp"

#include <memory>
#include <string>
#include <utility>

#include <glm/vec2.hpp>
#include <json.hpp>

#include "shim/propagate_const.hpp"

namespace block_thingy::graphics::gui::widget {

class Container : public Base
{
public:
	Container(Base* parent);
	~Container() override;

	std::string type() const override;

	void draw() override;

	void keypress(const input::key_press&) override;
	void charpress(const input::char_press&) override;
	void mousepress(const input::mouse_press&) override;
	void mousemove(const glm::dvec2& position) override;

	glm::dvec2 get_size() const override;

	void read_layout(const json&) override;
	void apply_layout(rhea::simplex_solver&, Container& root, Base::style_vars_t& window_vars) override;
	void use_layout() override;

	template<typename T, typename... Args>
	T& emplace_back(Args&&... args)
	{
		Base* new_widget = add_back(std::make_unique<T>(this, std::forward<Args>(args)...));
		return *dynamic_cast<T*>(new_widget);
	}

	Base* get_widget_by_id(const std::string& id);

	template<typename T>
	T* get_widget_by_id_t(const std::string& id)
	{
		return dynamic_cast<T*>(get_widget_by_id(id));
	}

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;

	Base* add_back(std::unique_ptr<Base>);
};

}
