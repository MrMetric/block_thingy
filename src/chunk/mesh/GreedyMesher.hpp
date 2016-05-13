#pragma once
#include "ChunkMesher.hpp"

#include <vector>

#include "block/BlockType.hpp"

class GreedyMesher : public ChunkMesher
{
	public:
		GreedyMesher();

		meshmap_t make_mesh(const Chunk&) override;

	private:
		std::vector<std::vector<BlockType>> surface;
};
