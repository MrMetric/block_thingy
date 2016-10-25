#pragma once
#include "Base.hpp"

namespace Block {

class None : public Base
{
	public:
		None(BlockType);

		BlockVisibilityType visibility_type() const override;
		glm::dvec4 selection_color() const override;
};

} // namespace Block
