#pragma once
#include "ChunkMesher.hpp"

#include <array>

#include "block/BlockType.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/Color.hpp"

class GreedyMesher : public ChunkMesher
{
	public:
		meshmap_t make_mesh(const Chunk&) override;

		using surface_t = std::array<std::array<meshmap_key_t, CHUNK_SIZE>, CHUNK_SIZE>;

	private:
		surface_t surface;
};
