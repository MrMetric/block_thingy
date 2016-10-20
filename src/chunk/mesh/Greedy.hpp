#pragma once
#include "Base.hpp"

#include <array>

#include "chunk/Chunk.hpp"

namespace Mesher {

class Greedy : public Base
{
	public:
		meshmap_t make_mesh(const Chunk&) override;

		using surface_t = std::array<std::array<meshmap_key_t, CHUNK_SIZE>, CHUNK_SIZE>;

	private:
		surface_t surface;
};

}
