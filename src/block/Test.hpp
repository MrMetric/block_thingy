#pragma once
#include "Base.hpp"

namespace Block {

class Test : public Base
{
public:
	Test(Enum::Type);

	double bounciness() const override;
	glm::dvec4 selection_color() const override;
};

} // namespace Block
