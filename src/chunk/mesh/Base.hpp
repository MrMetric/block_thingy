#pragma once

#include <map>
#include <stdint.h>
#include <tuple>
#include <vector>

#include <glad/glad.h>

#include "fwd/block/Base.hpp"
#include "fwd/block/BlockType.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "fwd/graphics/Color.hpp"
#include "graphics/primitive.hpp"

namespace Mesher {

using mesh_triangle_t = triangle_t<GLubyte>;
using mesh_vertex_coord_t = mesh_triangle_t::value_type;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_key_t = std::tuple<BlockType, Graphics::Color>;
using meshmap_t = std::map<meshmap_key_t, mesh_t>;

class Base
{
	public:
		static const meshmap_key_t empty_key;

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
