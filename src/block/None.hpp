#pragma once
#include "Base.hpp"

namespace Block {

class None : public Base
{
	public:
		None();

		BlockVisibilityType visibility_type() const override;
};

} // namespace Block
