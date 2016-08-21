#include "WidgetContainer.hpp"

#include <stdexcept>
#include <string>

namespace Graphics {
namespace GUI {

WidgetContainer::WidgetContainer(Game& game, const WidgetContainerMode mode)
	:
	game(game),
	mode(mode)
{
}

void WidgetContainer::draw()
{
	switch(mode)
	{
		case WidgetContainerMode::widgets:
		{
			for(auto& widget : widgets)
			{
				if(widget == nullptr) continue; // this can happen when the GUI closes
				widget->draw();
			}
			break;
		}
		case WidgetContainerMode::rows:
		case WidgetContainerMode::cols:
		{
			for(auto& container : containers)
			{
				if(container == nullptr) continue; // this can happen when the GUI closes
				container->draw();
			}
			break;
		}
	}
}

void WidgetContainer::mousepress(const int button, const int action, const int mods)
{
	switch(mode)
	{
		case WidgetContainerMode::widgets:
		{
			for(auto& widget : widgets)
			{
				if(widget == nullptr) continue; // this can happen when the GUI closes
				widget->mousepress(button, action, mods);
			}
			break;
		}
		case WidgetContainerMode::rows:
		case WidgetContainerMode::cols:
		{
			for(auto& container : containers)
			{
				if(container == nullptr) continue; // this can happen when the GUI closes
				container->mousepress(button, action, mods);
			}
			break;
		}
	}
}

void WidgetContainer::mousemove(const double x, const double y)
{
	switch(mode)
	{
		case WidgetContainerMode::widgets:
		{
			for(auto& widget : widgets)
			{
				if(widget == nullptr) continue; // this can happen when the GUI closes
				widget->mousemove(x, y);
			}
			break;
		}
		case WidgetContainerMode::rows:
		case WidgetContainerMode::cols:
		{
			for(auto& container : containers)
			{
				if(container == nullptr) continue; // this can happen when the GUI closes
				container->mousemove(x, y);
			}
			break;
		}
	}
}

void WidgetContainer::update_container(const glm::dvec2& position, const glm::dvec2& size)
{
	this->position = position;
	this->size = size;
	update_children();
}

WidgetContainer& WidgetContainer::add(WidgetContainerMode mode)
{
	if(this->mode != WidgetContainerMode::rows && this->mode != WidgetContainerMode::cols)
	{
		throw std::runtime_error(std::string("can not add container(")
								+ std::to_string(static_cast<int>(mode))
								+ ") to container("
								+ std::to_string(static_cast<int>(this->mode))
								+ ")");
	}

	containers.emplace_back(std::make_unique<WidgetContainer>(game, mode));
	update_children();
	return *containers.back();
}

void WidgetContainer::update_children()
{
	switch(mode)
	{
		case WidgetContainerMode::widgets:
		{
			if(widgets.size() == 0)
			{
				break;
			}

			const double spacing = 8;
			double total_height = 0;
			for(auto& widget : widgets)
			{
				total_height += widget->get_size().y;
			}
			total_height += spacing * (widgets.size() - 1);
			double y = -total_height / 2;
			for(auto& widget : widgets)
			{
				const double o = widget->get_size().y * widget->get_origin().y;
				widget->update_container(position, size, {0, y + o});
				y += widget->get_size().y + spacing;
			}
			break;
		}
		case WidgetContainerMode::rows:
		{
			if(containers.size() == 0)
			{
				return;
			}
			double row_height = size.y / containers.size();
			for(size_t i = 0; i < containers.size(); ++i)
			{
				if(containers[i] == nullptr) continue; // this can happen when the GUI closes
				containers[i]->update_container({position.x, position.y + i * row_height}, {size.x, row_height});
			}
			break;
		}
		case WidgetContainerMode::cols:
		{
			if(containers.size() == 0)
			{
				break;
			}
			const double col_width = size.x / containers.size();
			for(size_t i = 0; i < containers.size(); ++i)
			{
				if(containers[i] == nullptr) continue; // this can happen when the GUI closes
				containers[i]->update_container({position.x + i * col_width, position.y}, {col_width, size.y});
			}
			break;
		}
	}
}

} // namespace GUI
} // namespace Graphics
