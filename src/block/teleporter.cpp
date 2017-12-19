#include "teleporter.hpp"

#include "block/enums/visibility_type.hpp"

namespace block_thingy::block {

test_teleporter::test_teleporter(const enums::type t)
:
	base(t, enums::visibility_type::translucent, "teleporter")
{
}

bool test_teleporter::is_solid() const
{
	return false;
}

}
