#include "SimpleAO.hpp"

#include <functional>
#include <unordered_map>

#include "Cube.hpp"
#include "Game.hpp"
#include "block/Base.hpp"
#include "block/Enum/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

using Block::Enum::Face;
using Position::BlockInChunk;

namespace Mesher {

using i8vec3 = glm::tvec3<int8_t>;

static uint8_t o_key(i8vec3 o)
{
	o += 1;
	return static_cast<uint8_t>
	(
		   (o.x     )
		 | (o.y << 2)
		 | (o.z << 4)
	);
}

meshmap_t SimpleAO::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;
	for(BlockInChunk::value_type x = 0; x < CHUNK_SIZE; ++x)
	for(BlockInChunk::value_type y = 0; y < CHUNK_SIZE; ++y)
	for(BlockInChunk::value_type z = 0; z < CHUNK_SIZE; ++z)
	{
		const Block::Base& block = block_at(chunk, x, y, z);
		if(block.is_invisible())
		{
			continue;
		}

		std::unordered_map<i8vec3, glm::vec3, std::function<uint8_t(i8vec3)>> light_cache(0, o_key);
		auto l = [&chunk, x, y, z, &light_cache](const i8vec3& o) -> glm::vec3
		{
			const auto i = light_cache.find(o);
			if(i != light_cache.cend())
			{
				return i->second;
			}
			const glm::vec3 light = static_cast<glm::vec3>(light_at(chunk, x + o.x, y + o.y, z + o.z));
			light_cache.emplace(o, light);
			return light;
		};

		auto add_face =
		[
			&chunk,
			&meshes,
			x, y, z,
			&block,
			&l
		]
		(Face face)
		{
			const Side side = to_side(face);
			const auto i = get_i(face);
			i8vec3 x0_z0;
			x0_z0[i.y] = static_cast<int8_t>(side);
			if(block_visible_from(chunk, block, x + x0_z0.x, y + x0_z0.y, z + x0_z0.z))
			{
				i8vec3 xn_z0 = x0_z0;
				xn_z0[i.z] = -1;

				i8vec3 x0_zn = x0_z0;
				x0_zn[i.x] = -1;

				i8vec3 xn_zn = x0_z0;
				xn_zn[i.z] = -1;
				xn_zn[i.x] = -1;

				i8vec3 xp_z0 = x0_z0;
				xp_z0[i.z] = +1;

				i8vec3 x0_zp = x0_z0;
				x0_zp[i.x] = +1;

				i8vec3 xp_zn = x0_z0;
				xp_zn[i.z] = +1;
				xp_zn[i.x] = -1;

				i8vec3 xp_zp = x0_z0;
				xp_zp[i.z] = +1;
				xp_zp[i.x] = +1;

				i8vec3 xn_zp = x0_z0;
				xn_zp[i.z] = -1;
				xn_zp[i.x] = +1;

				glm::tvec4<glm::vec3> light =
				{
					(l(x0_z0) + l(x0_zn) + l(xn_zn) + l(xn_z0)) / 4.0f,
					(l(x0_z0) + l(x0_zp) + l(xn_zp) + l(xn_z0)) / 4.0f,
					(l(x0_z0) + l(x0_zp) + l(xp_zp) + l(xp_z0)) / 4.0f,
					(l(x0_z0) + l(x0_zn) + l(xp_zn) + l(xp_z0)) / 4.0f,
				};
				const auto tex = Game::instance->resource_manager.get_block_texture(block.texture(face));
				const meshmap_key_t key =
				{
					block.type(),
					tex.unit,
				};
				Base::add_face(meshes[key], {x, y, z}, face, 1, 1, light, tex.index);
			}
		};
		add_face(Face::right);
		add_face(Face::left);
		add_face(Face::top);
		add_face(Face::bottom);
		add_face(Face::front);
		add_face(Face::back);
	}
	return meshes;
}

}
