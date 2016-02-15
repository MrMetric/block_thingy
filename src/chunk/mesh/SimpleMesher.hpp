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

		mesh_t make_mesh() override;

	private:
		void draw_cube(std::vector<GLubyte>&, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
		void draw_face(std::vector<GLubyte>&, BlockInChunk_type, BlockInChunk_type, BlockInChunk_type, uint_fast8_t face);
};
