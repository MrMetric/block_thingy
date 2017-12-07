#pragma once
#include "Base.hpp"

namespace Block {

class Air : public Base
{
public:
	Air(Enum::Type);

	bool is_solid() const override;
	bool is_selectable() const override;
	bool is_replaceable_by(const Base&) const override;
};

}
