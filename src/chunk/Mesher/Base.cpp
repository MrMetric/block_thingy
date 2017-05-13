#include "Base.hpp"

#include <cassert>
#include <stdint.h>

#include "Game.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "block/Enum/Face.hpp"
#include "chunk/Chunk.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"

using Block::Enum::Face;

namespace Mesher {

mesh_vertex_t::mesh_vertex_t()
{
}

mesh_vertex_t::mesh_vertex_t(uint8_t)
{
}

Base::Base()
{
}

Base::~Base()
{
}

static void add_square
(
	mesh_t& mesh,
	const mesh_vertex_t& p1,
	const mesh_vertex_t& p2,
	const mesh_vertex_t& p3,
	const mesh_vertex_t& p4
)
{
	mesh.emplace_back(p1, p2, p3);
	mesh.emplace_back(p3, p4, p1);
}

void Base::add_face
(
	mesh_t& mesh,
	u8vec3 xyz,
	const Face face,
	const uint8_t offset_x,
	const uint8_t offset_z,
	const glm::tvec4<glm::vec3>& light
)
{
	const u8vec3 i = get_i(face);

	const Side side = to_side(face);
	if(side == Side::top)
	{
		xyz[i.y] += 1;
	}

	u8vec3 mod2;
	mod2[i.x] = offset_x;

	u8vec3 mod3;
	mod3[i.x] = offset_x;
	mod3[i.z] = offset_z;

	u8vec3 mod4;
	mod4[i.z] = offset_z;

	mesh_vertex_t v1, v2, v3, v4;
	v1.pos = xyz;
	v2.pos = xyz + mod2;
	v3.pos = xyz + mod3;
	v4.pos = xyz + mod4;
	v1.light = v2.light = v3.light = v4.light = light;
	v1.face = v2.face = v3.face = v4.face = face;

	if(side == Side::top)
	{
		add_square(mesh, v1, v2, v3, v4);
	}
	else
	{
		add_square(mesh, v4, v3, v2, v1);
	}
}

void Base::add_face
(
	mesh_t& mesh,
	u8vec3 xyz,
	const Block::Enum::Face face,
	const uint8_t offset_x,
	const uint8_t offset_z,
	const glm::vec3& light
)
{
	add_face(mesh, xyz, face, offset_x, offset_z, glm::tvec4<glm::vec3>(light));
}

u8vec3 Base::get_i(const Face face)
{
	if(face == Face::front || face == Face::back)
	{
		return {0, 2, 1};
	}
	else if(face == Face::top || face == Face::bottom)
	{
		return {2, 1, 0};
	}
	else
	{
		assert(face == Face::right || face == Face::left);
		return {1, 0, 2};
	}
}

Side Base::to_side(const Face face)
{
	return (face == Face::top || face == Face::back || face == Face::left) ? Side::top : Side::bottom;
}

const Block::Base& Base::block_at
(
	const Chunk& chunk,
	const int_fast16_t x,
	const int_fast16_t y,
	const int_fast16_t z
)
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		Position::BlockInWorld block_pos(chunk.get_position(), {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_block(block_pos);
	}
	#define s(a) static_cast<Position::BlockInChunk::value_type>(a)
	return chunk.get_block({s(x), s(y), s(z)});
	#undef s
}

bool Base::block_visible_from
(
	const Chunk& chunk,
	const Block::Base& block,
	const int_fast16_t x,
	const int_fast16_t y,
	const int_fast16_t z
)
{
	const Block::Base& sibling = block_at(chunk, x, y, z);
	return
		   sibling.type() != BlockType::none
		&& !block.is_invisible() // this block is visible
		&& !sibling.is_opaque() // this block can be seen thru the adjacent block
		&& block.type() != sibling.type() // do not show sides inside of adjacent translucent blocks (of the same type)
	;
}

Graphics::Color Base::light_at
(
	const Chunk& chunk,
	const int_fast16_t x,
	const int_fast16_t y,
	const int_fast16_t z
)
{
	if(x < 0 || x >= CHUNK_SIZE
	|| y < 0 || y >= CHUNK_SIZE
	|| z < 0 || z >= CHUNK_SIZE)
	{
		Position::ChunkInWorld chunk_pos = chunk.get_position();
		Position::BlockInWorld block_pos(chunk_pos, {0, 0, 0});
		block_pos.x += x;
		block_pos.y += y;
		block_pos.z += z;
		return chunk.get_owner().get_light(block_pos);
	}
	#define s(a) static_cast<Position::BlockInChunk::value_type>(a)
	return chunk.get_light({ s(x), s(y), s(z) });
	#undef s
}

}
