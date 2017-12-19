#include "air.hpp"

#include "block/enums/visibility_type.hpp"

namespace block_thingy::block {

air::air(const enums::type t)
:
	base(t, enums::visibility_type::invisible)
{
}

bool air::is_solid() const
{
	return false;
}

bool air::is_selectable() const
{
	return false;
}

bool air::is_replaceable_by(const base&) const
{
	return true;
}

}
