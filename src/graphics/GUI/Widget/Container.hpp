#pragma once
#include "Base.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec2.hpp>
#include <json.hpp>

#include "fwd/Game.hpp"
#include "fwd/util/key_mods.hpp"

#include "std_make_unique.hpp"

namespace Graphics::GUI::Widget {

class Container : public Base
{
	public:
		Container();

		std::string type() const override;

		void draw() override;

		void keypress(int key, int scancode, int action, Util::key_mods) override;
		void charpress(char32_t, Util::key_mods) override;
		void mousepress(int button, int action, Util::key_mods) override;
		void mousemove(double x, double y) override;

		glm::dvec2 get_size() const override;

		void read_layout(const json&) override;
		void apply_layout(rhea::simplex_solver&, Base::style_vars_t& parent_vars) override;
		void use_layout() override;

		template<typename T, typename... Args>
		T& add(Args&&... args)
		{
			widgets.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			// TODO?: apply_layout(solver);
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
};

}
