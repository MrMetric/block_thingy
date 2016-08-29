#pragma once
#include "Block.hpp"

namespace Block {

class Test : public Block
{
	public:
		Test();

		double bounciness() const override;
};

} // namespace Block
