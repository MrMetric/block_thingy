#pragma once
#include "Base.hpp"

namespace Mesher {

class SimpleAO : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;
};

}
