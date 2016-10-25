#pragma once
#include "Base.hpp"

namespace Block {

class Glass : public Base
{
	public:
		Glass(BlockType);

		BlockVisibilityType visibility_type() const;
};

}
