#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class none : public base
{
public:
	none(enums::type);

	glm::dvec4 selection_color() const override;
};

}
