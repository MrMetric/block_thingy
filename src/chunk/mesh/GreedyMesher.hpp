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
		std::vector<std::vector<block_id_type>> surface;

		GreedyMesherPrivate::Rectangle yield_rectangle();
		void add_surface(std::vector<GLubyte>& vertexes, const GreedyMesherPrivate::Plane plane, const GreedyMesherPrivate::Side side);
};
