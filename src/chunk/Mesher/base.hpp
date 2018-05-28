#pragma once

#include <map>
#include <stdint.h>
#include <tuple>
#include <vector>

#include <glm/vec3.hpp>

#include "block/block.hpp"
#include "fwd/block/component/info.hpp"
#include "fwd/block/enums/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "graphics/primitive.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::mesher {

using u8vec3 = glm::tvec3<uint8_t>;

#pragma pack(push, 1)
struct mesh_vertex_t
{
	mesh_vertex_t();

	mesh_vertex_t
	(
		const vertex_coord_t<uint8_t>& pos,
		block::enums::Face,
		uint8_t rotation,
		uint16_t tex_index
	);

	vertex_coord_t<uint8_t> pos;
	uint8_t face_and_rotation;
	uint16_t tex_index;
};
#pragma pack(pop)

struct meshmap_key_t
{
	fs::path shader_path;
	bool is_translucent;
	uint8_t tex_unit;

	bool operator==(const meshmap_key_t& that) const
	{
		return
			shader_path == that.shader_path
		 && is_translucent == that.is_translucent
		 && tex_unit == that.tex_unit;
	}

	// for std::map
	bool operator<(const meshmap_key_t& that) const
	{
		return std::tie(shader_path, is_translucent, tex_unit)
			 < std::tie(that.shader_path, that.is_translucent, that.tex_unit);
	}
};

using mesh_triangle_t = glm::tvec3<mesh_vertex_t>;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_t = std::map<meshmap_key_t, mesh_t>;

enum class Plane
{
	XY,
	XZ,
	YZ,
};

enum class Side : int8_t
{
	top    = +1,
	bottom = -1,
};

class base
{
public:
	base();
	virtual ~base();

	base(base&&) = delete;
	base(const base&) = delete;
	base& operator=(base&&) = delete;
	base& operator=(const base&) = delete;

	virtual meshmap_t make_mesh(const Chunk&) = 0;

	static void add_face
	(
		mesh_t& mesh,
		u8vec3 xyz,
		block::enums::Face face,
		uint8_t offset_x,
		uint8_t offset_z,
		uint16_t tex_index,
		uint8_t rotation
	);
	static u8vec3 get_i(block::enums::Face);

	static Side to_side(block::enums::Face);

	static block_t block_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
	static bool block_visible_from
	(
		const block::component::info&,
		const Chunk&,
		block_t,
		int_fast16_t x, int_fast16_t y, int_fast16_t z
	);
};

}
