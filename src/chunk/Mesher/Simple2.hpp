#pragma once
#include "Base.hpp"

namespace Mesher {

class Simple2 : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
