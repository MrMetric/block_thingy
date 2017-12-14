#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class None : public Base
{
public:
	None(enums::Type);

	glm::dvec4 selection_color() const override;
};

}
