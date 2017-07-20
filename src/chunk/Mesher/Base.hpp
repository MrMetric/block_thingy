#pragma once

#include <map>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "fwd/block/Base.hpp"
#include "fwd/block/Enum/Face.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "graphics/Color.hpp"
#include "graphics/primitive.hpp"

namespace Mesher {

using u8vec3 = glm::tvec3<uint8_t>;

#pragma pack(push, 1)
struct mesh_vertex_t
{
	mesh_vertex_t();
	mesh_vertex_t(uint8_t);

	mesh_vertex_t
	(
		const vertex_coord_t<uint8_t>& pos,
		Block::Enum::Face,
		uint8_t rotation,
		const glm::tvec4<u8vec3>& light,
		uint16_t tex_index
	);

	vertex_coord_t<uint8_t> pos;
	uint8_t face_and_rotation;
	glm::tvec4<u8vec3> light;
	uint16_t tex_index;
};
#pragma pack(pop)

struct meshmap_key_t
{
	Block::Enum::Type block_type;
	bool is_translucent;
	uint8_t tex_unit;

	bool operator==(const meshmap_key_t& that) const
	{
		return
			block_type == that.block_type
		 && is_translucent == that.is_translucent
		 && tex_unit == that.tex_unit;
	}

	bool operator<(const meshmap_key_t& that) const
	{
		return block_type < that.block_type;
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

class Base
{
public:
	Base();
	virtual ~Base();

	Base(Base&&) = delete;
	Base(const Base&) = delete;
	void operator=(const Base&) = delete;

	virtual meshmap_t make_mesh(const Chunk&) = 0;

	static void add_face
	(
		mesh_t& mesh,
		u8vec3 xyz,
		Block::Enum::Face face,
		uint8_t offset_x,
		uint8_t offset_z,
		const glm::tvec4<u8vec3>& light,
		uint16_t tex_index,
		uint8_t rotation
	);
	static void add_face
	(
		mesh_t& mesh,
		u8vec3 xyz,
		Block::Enum::Face face,
		uint8_t offset_x,
		uint8_t offset_z,
		const u8vec3& light,
		uint16_t tex_index,
		uint8_t rotation
	);
	static u8vec3 get_i(Block::Enum::Face);

	static Side to_side(Block::Enum::Face);

	static const Block::Base& block_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
	static bool block_visible_from(const Chunk&, const Block::Base&, int_fast16_t, int_fast16_t, int_fast16_t);
	static Graphics::Color light_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
};

}
