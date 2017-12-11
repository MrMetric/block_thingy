#pragma once
#include "Base.hpp"

namespace Block {

class Light : public Base
{
public:
	Light(Enum::Type);
	Light(Enum::Type, const Graphics::Color&);

	std::string name() const override;

	void use_start(Game&, World&, Player&, const Position::BlockInWorld&, Enum::Face) override;

	void save(Storage::OutputInterface&) const override;
	void load(Storage::InputInterface&) override;
};

}
