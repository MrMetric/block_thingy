#pragma once
#include "Base.hpp"

namespace block_thingy::block {

class Teleporter : public Base
{
public:
	Teleporter(enums::Type);

	bool is_solid() const override;
};

}
