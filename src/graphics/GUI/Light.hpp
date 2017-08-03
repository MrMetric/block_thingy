#pragma once
#include "Base.hpp"

#include <string>

#include "fwd/Game.hpp"
#include "fwd/World.hpp"
#include "fwd/block/Light.hpp"
#include "fwd/graphics/GUI/Widget/TextInput.hpp"
#include "position/BlockInWorld.hpp"

namespace Graphics::GUI {

class Light : public Base
{
public:
	Light(Game&, World&, Block::Light&, const Position::BlockInWorld&);

	std::string type() const override;

	void init() override;
	void draw() override;

private:
	World& world;
	Block::Light& block;
	Position::BlockInWorld block_pos;

	void on_change(uint_fast8_t, Widget::TextInput&, const std::string&);
};

}
