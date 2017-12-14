#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class Air : public Base
{
public:
	Air(enums::Type);

	bool is_solid() const override;
	bool is_selectable() const override;
	bool is_replaceable_by(const Base&) const override;
};

}
