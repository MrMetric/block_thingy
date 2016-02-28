#pragma once

#include <map>
#include <vector>

#include <glad/glad.h>

#include <glm/vec3.hpp>

#include "../../BlockType.hpp"

class Block;
class Chunk;

using mesh_vertex_coord_t = glm::tvec3<GLubyte>;
using mesh_triangle_t = glm::tvec3<mesh_vertex_coord_t>;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_t = std::map<BlockType, mesh_t>;

class ChunkMesher
{
	public:
		explicit ChunkMesher(const Chunk&);
		virtual ~ChunkMesher();

		ChunkMesher(const ChunkMesher&) = delete;
		ChunkMesher& operator=(const ChunkMesher&) = delete;

		virtual meshmap_t make_mesh() = 0;

	protected:
		const Chunk& chunk;

		const Block& block_at(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_invisible(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
		bool block_is_invisible_not_none(int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true) const;
};
