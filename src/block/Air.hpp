#pragma once
#include "Block.hpp"

namespace Block {

class Air : public Block
{
	public:
		Air();

		BlockVisibilityType visibility_type() const override;
		bool is_solid() const override;
		bool is_selectable() const override;
};

} // namespace Block
