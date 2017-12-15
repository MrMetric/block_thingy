#pragma once
#include "Base.hpp"

#include <cstddef>
#include <string>

#include "fwd/Game.hpp"
#include "fwd/World.hpp"
#include "fwd/block/Light.hpp"
#include "fwd/graphics/GUI/Widget/TextInput.hpp"
#include "position/BlockInWorld.hpp"

namespace block_thingy::graphics::gui {

class Light : public Base
{
public:
	Light(Game&, World&, block::Light&, const position::BlockInWorld&);

	std::string type() const override;

	void init() override;
	void draw() override;

private:
	World& world;
	block::Light& block;
	position::BlockInWorld block_pos;

	void on_change(std::ptrdiff_t, widget::TextInput&, const std::string&);
};

}
