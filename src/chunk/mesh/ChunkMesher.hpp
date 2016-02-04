#pragma once

#include <vector>

#include <GL/glew.h>

class Chunk;

class ChunkMesher
{
	public:
		ChunkMesher(const Chunk&);
		ChunkMesher(const ChunkMesher&) = delete;
		virtual ~ChunkMesher();

		virtual std::vector<GLfloat> make_mesh() = 0;

	protected:
		const Chunk& chunk;
};