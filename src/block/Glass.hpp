#pragma once
#include "Base.hpp"

namespace Block {

class Glass : public Base
{
public:
	Glass(Enum::Type);

	Enum::VisibilityType visibility_type() const;
};

}
