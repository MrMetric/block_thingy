#pragma once
#include "ChunkMesher.hpp"

#include <array>

#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"

class GreedyMesher : public ChunkMesher
{
	public:
		meshmap_t make_mesh(const Chunk&) override;

		using surface_t = std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE>;

	private:
		surface_t surface;
};
