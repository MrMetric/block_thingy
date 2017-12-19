#pragma once
#include "base.hpp"

namespace block_thingy::block {

class test_light : public base
{
public:
	test_light(enums::type);
	test_light(enums::type, const graphics::color&);

	std::string name() const override;

	void use_start(game&, world::world&, Player&, const position::block_in_world&, enums::Face) override;

	void save(storage::OutputInterface&) const override;
	void load(storage::InputInterface&) override;
};

}
