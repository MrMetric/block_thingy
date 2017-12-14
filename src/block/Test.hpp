#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class Test : public Base
{
public:
	Test(enums::Type);

	double bounciness() const override;
	glm::dvec4 selection_color() const override;
};

}
