#pragma once
#include "base.hpp"

namespace block_thingy::mesher {

class simple2 : public base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
