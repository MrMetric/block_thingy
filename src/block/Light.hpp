#pragma once
#include "Base.hpp"
#include "Interface/KnowsPosition.hpp"

#include "fwd/World.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInWorld.hpp"

namespace Block {

class Light : public Base, public Interface::KnowsPosition
{
	public:
		Light(BlockType);
		Light(BlockType, const Graphics::Color&);

		Light& operator=(const Base&) override;

		Graphics::Color color() const override;
		void color(const Graphics::Color&);

		void use_start() override;

		void save(Storage::OutputInterface&) const override;
		void load(Storage::InputInterface&) override;

	private:
		Graphics::Color color_;
};

}
