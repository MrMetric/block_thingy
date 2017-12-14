#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class Light : public Base
{
public:
	Light(enums::Type);
	Light(enums::Type, const graphics::Color&);

	std::string name() const override;

	void use_start(Game&, World&, Player&, const position::BlockInWorld&, enums::Face) override;

	void save(storage::OutputInterface&) const override;
	void load(storage::InputInterface&) override;
};

}
