#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class test : public base
{
public:
	test(enums::type);

	double bounciness() const override;
	glm::dvec4 selection_color() const override;
};

}
