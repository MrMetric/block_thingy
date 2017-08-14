#pragma once
#include "SimpleShader.hpp"

namespace Block {

class Glass : public SimpleShader
{
public:
	Glass(Enum::Type);

	Enum::VisibilityType visibility_type() const;
};

}
