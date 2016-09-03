#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glm/vec2.hpp>

#include "fwd/Game.hpp"
#include "graphics/GUI/Widget/Base.hpp"

#include "std_make_unique.hpp"

namespace Graphics::GUI {

enum class WidgetContainerMode
{
	widgets,
	rows,
	cols,
};

class WidgetContainer
{
	public:
		WidgetContainer(Game&, WidgetContainerMode mode);

		void draw();

		void mousepress(int button, int action, int mods);
		void mousemove(double x, double y);

		void update_container(const glm::dvec2& position, const glm::dvec2& size);

		template <typename T, typename... Args>
		T& add(Args&&... args)
		{
			if(this->mode != WidgetContainerMode::widgets)
			{
				throw std::runtime_error(std::string("can not add widget to container(")
										+ std::to_string(static_cast<int>(this->mode))
										+ ")");
			}

			widgets.emplace_back(std::make_unique<T>(*this, std::forward<Args>(args)...));
			update_children();
			return *dynamic_cast<T*>(widgets.back().get());
		}

		WidgetContainer& add(WidgetContainerMode mode);

		Game& game;

	private:
		void update_children();

		glm::dvec2 position;
		glm::dvec2 size;

		const WidgetContainerMode mode;
		std::vector<std::unique_ptr<Widget::Base>> widgets;
		std::vector<std::unique_ptr<WidgetContainer>> containers;
};

} // namespace Graphics::GUI
