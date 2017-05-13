#pragma once
#include "Base.hpp"

#include <array>
#include <tuple>

#include "fwd/chunk/Chunk.hpp"

namespace Mesher {

class Greedy : public Base
{
public:
	meshmap_t make_mesh(const Chunk&) override;

	using surface_t = std::array<std::array<std::tuple<BlockType, Graphics::Color>, CHUNK_SIZE>, CHUNK_SIZE>;

private:
	surface_t surface;
};

}
