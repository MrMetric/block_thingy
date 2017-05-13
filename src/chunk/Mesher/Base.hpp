#pragma once

#include <map>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
	Block::Enum::Face face;
	glm::tvec4<glm::vec3> light;
};
#pragma pack(pop)

using mesh_triangle_t = glm::tvec3<mesh_vertex_t>;
using mesh_t = std::vector<mesh_triangle_t>;
using meshmap_t = std::map<BlockType, mesh_t>;
using u8vec3 = glm::tvec3<uint8_t>;

enum class Plane
{
	XY,
	XZ,
	YZ,
};

enum class Side : int8_t
{
	top = 1,
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
		const Block::Enum::Face face,
		const uint8_t offset_x,
		const uint8_t offset_z,
		const glm::tvec4<glm::vec3>& light
	);
	static void add_face
	(
		mesh_t& mesh,
		u8vec3 xyz,
		const Block::Enum::Face face,
		const uint8_t offset_x,
		const uint8_t offset_z,
		const glm::vec3& light
	);
	static u8vec3 get_i(Block::Enum::Face);

	static Side to_side(Block::Enum::Face);

	static const Block::Base& block_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
	static bool block_visible_from(const Chunk&, const Block::Base&, int_fast16_t, int_fast16_t, int_fast16_t);
	static Graphics::Color light_at(const Chunk&, int_fast16_t x, int_fast16_t y, int_fast16_t z);
};

}
