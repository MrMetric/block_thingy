#pragma once
#include "Base.hpp"

namespace Block {

class None : public Base
{
public:
	None(Enum::Type);

	glm::dvec4 selection_color() const override;
};

}
