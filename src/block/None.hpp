#pragma once
#include "Block.hpp"

namespace Block {

class None : public Block
{
	public:
		None();

		BlockVisibilityType visibility_type() const override;
};

} // namespace Block
