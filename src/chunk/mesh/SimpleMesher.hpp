#pragma once
#include "ChunkMesher.hpp"

class SimpleMesher : public ChunkMesher
{
	public:
		meshmap_t make_mesh(const Chunk&) override;
};
