#pragma once
#include "Base.hpp"

#include <cstddef>
#include <string>

#include "fwd/game.hpp"
#include "block/block.hpp"
#include "fwd/graphics/GUI/Widget/text_input.hpp"
#include "position/block_in_world.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::graphics::gui {

class light : public Base
{
public:
	light(game&, world::world&, const position::block_in_world&, block_t);

	std::string type() const override;

	void draw() override;

private:
	world::world& world;
	position::block_in_world block_pos;
	block_t block;

	void on_change(std::ptrdiff_t, widget::text_input&, const std::string&);
};

}
