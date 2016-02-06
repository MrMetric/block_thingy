#include "ChunkMesher.hpp"

class GreedyMesher : public ChunkMesher
{
	public:
		explicit GreedyMesher(const Chunk&);

		std::vector<GLbyte> make_mesh();
};
