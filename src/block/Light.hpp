#pragma once
#include "Block.hpp"

#include "graphics/Color.hpp"

namespace Block {

class Light : public Block
{
	public:
		Light();
		Light(const Graphics::Color&);

		void operator=(const Block&) override;

		Graphics::Color color() const override;

	private:
		Graphics::Color color_;
};

}
