#pragma once
#include "base.hpp"

namespace block_thingy::block {

class none : public base
{
public:
	none(enums::type);

	glm::dvec4 selection_color() const override;
};

}
