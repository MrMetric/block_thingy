#pragma once
#include "ChunkMesher.hpp"

#include <cstdint>
#include <vector>

#include <glm/vec3.hpp>

#include "BlockType.hpp"

namespace GreedyMesherPrivate
{
	struct Rectangle;
	enum class Plane;
	enum class Side : int_fast8_t;
}

class GreedyMesher : public ChunkMesher
{
	public:
		explicit GreedyMesher(const Chunk&);

		meshmap_t make_mesh() override;

	private:
		std::vector<std::vector<BlockType>> surface;

		void add_surface(meshmap_t&, GreedyMesherPrivate::Plane, GreedyMesherPrivate::Side);
		void generate_surface(glm::tvec3<uint_fast8_t>&, uint_fast8_t, uint_fast8_t, uint_fast8_t, int_fast8_t);
		GreedyMesherPrivate::Rectangle yield_rectangle();
		void add_face(const mesh_vertex_coord_t& p1, const mesh_vertex_coord_t& p2, const mesh_vertex_coord_t& p3, const mesh_vertex_coord_t& p4, mesh_t& vertexes);
};
