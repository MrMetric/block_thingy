#pragma once
#include "Base.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec2.hpp>
#include <json.hpp>

namespace block_thingy::graphics::gui::widget {

class Container : public Base
{
public:
	Container(Base* parent);

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
		widgets.emplace_back(std::make_unique<T>(this, std::forward<Args>(args)...));
		add_back();
		return *dynamic_cast<T*>(widgets.back().get());
	}

	template<typename T>
	T* get_widget_by_id(const std::string& id)
	{
		for(auto& w : widgets)
		{
			if(w->id == id)
			{
				return dynamic_cast<T*>(w.get());
			}
			Container* c = dynamic_cast<Container*>(w.get());
			if(c != nullptr)
			{
				T* w2 = c->get_widget_by_id<T>(id);
				if(w2 != nullptr)
				{
					return w2;
				}
			}
		}
		return nullptr;
	}

private:
	std::vector<std::unique_ptr<Base>> widgets;
	void add_back();
};

}
