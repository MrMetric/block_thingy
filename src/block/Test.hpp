#pragma once
#include "Base.hpp"

namespace Block {

class Test : public Base
{
	public:
		Test();

		double bounciness() const override;
};

} // namespace Block
