#pragma once
#include "Base.hpp"

#include "graphics/Color.hpp"

namespace Block {

class Light : public Base
{
	public:
		Light(Enum::Type);
		Light(Enum::Type, const Graphics::Color&);

		Light& operator=(const Base&) override;

		std::string name() const override;

		Graphics::Color color() const override;
		void color(const Graphics::Color&);

		void use_start(const Position::BlockInWorld&, Enum::Face) override;

		void save(Storage::OutputInterface&) const override;
		void load(Storage::InputInterface&) override;

	private:
		Graphics::Color color_;
};

}
