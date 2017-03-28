#pragma once
#include "Base.hpp"

#include <string>

#include "fwd/Game.hpp"
#include "fwd/block/Light.hpp"
#include "fwd/graphics/GUI/Widget/TextInput.hpp"

namespace Graphics::GUI {

class Light : public Base
{
	public:
		Light(Game&, Block::Light&);

		std::string type() const override;

		void init() override;
		void draw() override;

	private:
		Block::Light& block;

		void on_change(uint_fast8_t, Widget::TextInput&, const std::string&);
};

}
