#pragma once
#include "base.hpp"

namespace block_thingy::block {

class test_teleporter : public base
{
public:
	test_teleporter(enums::type);

	bool is_solid() const override;
};

}
