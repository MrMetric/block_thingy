#pragma once
#include "Base.hpp"

namespace block_thingy::mesher {

class Greedy : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
