#pragma once
#include "Base.hpp"

namespace Mesher {

class Simple : public Base
{
	public:
		meshmap_t make_mesh(const Chunk&) override;
};

}
