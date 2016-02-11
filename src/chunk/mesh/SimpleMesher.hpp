#pragma once
#include "ChunkMesher.hpp"

#include <cstdint>
#include <vector>

#include <glad/glad.h>

#include "../../Coords.hpp"

class SimpleMesher : public ChunkMesher
{
	public:
		explicit SimpleMesher(const Chunk&);

		std::vector<GLubyte> make_mesh();

	private:
		void draw_cube(std::vector<GLubyte>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		void draw_face(std::vector<GLubyte>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t face);
};
