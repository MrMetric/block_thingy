#pragma once
#include "Base.hpp"

#include "graphics/Color.hpp"

namespace Block {

class Light : public Base
{
	public:
		Light();
		Light(const Graphics::Color&);

		void operator=(const Base&) override;

		Graphics::Color color() const override;

	private:
		Graphics::Color color_;
};

}
