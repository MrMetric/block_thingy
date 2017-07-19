#pragma once
#include "Base.hpp"

namespace Block {

class Air : public Base
{
public:
	Air(Enum::Type);

	Enum::VisibilityType visibility_type() const override;
	bool is_solid() const override;
	bool is_selectable() const override;
	bool is_replaceable() const override;
};

} // namespace Block
