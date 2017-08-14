#pragma once
#include "SimpleShader.hpp"

namespace Block {

class Test : public SimpleShader
{
public:
	Test(Enum::Type);

	double bounciness() const override;
	glm::dvec4 selection_color() const override;
};

} // namespace Block
