#pragma once
#include "ChunkMesher.hpp"

#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "../../Coords.hpp"

class SimpleMesher : public ChunkMesher
{
	public:
		explicit SimpleMesher(const Chunk&);

		std::vector<GLfloat> make_mesh();

	private:
		void draw_cube(std::vector<GLfloat>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		void draw_face(std::vector<GLfloat>&, BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, uint_fast8_t face);
		bool block_is_empty(int_fast16_t x, int_fast16_t y, int_fast16_t z) const;
};