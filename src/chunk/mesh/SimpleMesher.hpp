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

		std::vector<GLbyte> make_mesh();

	private:
		void draw_cube(std::vector<GLbyte>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		void draw_face(std::vector<GLbyte>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t face);
};
