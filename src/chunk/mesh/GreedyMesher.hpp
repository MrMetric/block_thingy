#pragma once
#include "ChunkMesher.hpp"

#include <vector>

#include "../../BlockType.hpp"

namespace GreedyMesherPrivate
{
	struct Rectangle;
	enum class Plane;
	enum class Side : int_fast8_t;
}

class GreedyMesher : public ChunkMesher
{
	public:
		explicit GreedyMesher(const Chunk&);

		meshmap_t make_mesh() override;

	private:
		std::vector<std::vector<BlockType>> surface;

		GreedyMesherPrivate::Rectangle yield_rectangle();
		void add_surface(meshmap_t&, const GreedyMesherPrivate::Plane, const GreedyMesherPrivate::Side);
};
