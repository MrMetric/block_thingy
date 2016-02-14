#pragma once
#include "ChunkMesher.hpp"

#include <vector>

#include "../../BlockType.hpp"

namespace GreedyMesherPrivate
{
	struct Rectangle;
	enum class Plane;
	enum class Side;
}

class GreedyMesher : public ChunkMesher
{
	public:
		explicit GreedyMesher(const Chunk&);

		mesh_t make_mesh();

	private:
		std::vector<std::vector<BlockType>> surface;

		GreedyMesherPrivate::Rectangle yield_rectangle();
		void add_surface(mesh_t&, const GreedyMesherPrivate::Plane, const GreedyMesherPrivate::Side);
};
