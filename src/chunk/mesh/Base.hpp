#pragma once

#include <map>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>

#include "fwd/block/Base.hpp"
#include "fwd/block/BlockType.hpp"
#include "fwd/block/Enum/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "graphics/Color.hpp"
#include "graphics/primitive.hpp"

namespace Mesher {

#pragma pack(push, 1)
struct mesh_vertex_t
{
	mesh_vertex_t();
	mesh_vertex_t(uint8_t);

	vertex_coord_t<uint8_t> pos;
	Graphics::Color light;
	Block::Enum::Face face;
};
#pragma pack(pop)

using mesh_triangle_t = glm::tvec3<mesh_vertex_t>;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_key_t = BlockType;
using meshmap_t = std::map<meshmap_key_t, mesh_t>;

class Base
{
	public:
		Base();
		virtual ~Base();

		Base(Base&&) = delete;
		Base(const Base&) = delete;
		void operator=(const Base&) = delete;

		virtual meshmap_t make_mesh(const Chunk&) = 0;

		static const Block::Base& block_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z, bool allow_out_of_bounds = true);
		static bool block_visible_from(const Chunk&, const Block::Base&, int_fast16_t, int_fast16_t, int_fast16_t);
		static Graphics::Color light_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
};

}
