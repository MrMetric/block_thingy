#pragma once
#include "ChunkMesher.hpp"

class SimpleMesher : public ChunkMesher
{
	public:
		SimpleMesher();

		meshmap_t make_mesh(const Chunk&) override;
};
