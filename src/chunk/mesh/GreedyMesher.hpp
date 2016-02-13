#pragma once
#include "ChunkMesher.hpp"

#include <vector>

#include "../../Block.hpp"

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

		std::vector<GLubyte> make_mesh();

	private:
		std::vector<std::vector<BlockType>> surface;

		GreedyMesherPrivate::Rectangle yield_rectangle();
		void add_surface(std::vector<GLubyte>&, const GreedyMesherPrivate::Plane, const GreedyMesherPrivate::Side);
};
