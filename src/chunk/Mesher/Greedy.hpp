#pragma once
#include "Base.hpp"

namespace Mesher {

class Greedy : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
