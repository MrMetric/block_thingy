#pragma once
#include "Base.hpp"

namespace Block {

class Teleporter : public Base
{
public:
	Teleporter(Enum::Type);

	bool is_solid() const override;
};

}
