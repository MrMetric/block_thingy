#pragma once
#include "base.hpp"

namespace block_thingy::block {

class air : public base
{
public:
	air(enums::type);

	bool is_solid() const override;
	bool is_selectable() const override;
	bool is_replaceable_by(const base&) const override;
};

}
