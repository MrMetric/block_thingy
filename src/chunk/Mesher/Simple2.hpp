#pragma once
#include "Base.hpp"

namespace block_thingy::mesher {

class Simple2 : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
