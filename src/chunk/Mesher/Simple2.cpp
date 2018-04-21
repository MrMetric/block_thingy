#include "Simple2.hpp"

#include <array>
#include <cstddef>

#include "block/base.hpp"
#include "block/enums/type.hpp"
#include "chunk/Chunk.hpp"
#include "position/block_in_chunk.hpp"

namespace block_thingy::mesher {

using block::enums::Face;
using position::block_in_chunk;

meshmap_t simple2::make_mesh(const Chunk& chunk)
{
	meshmap_t meshes;

	std::array<const block::base*, CHUNK_BLOCK_COUNT> cache;

	std::size_t block_i = 0;
	block_in_chunk c_pos(0, 0, 0);
	for(c_pos.x = 0; c_pos.x < CHUNK_SIZE; ++c_pos.x)
	for(c_pos.y = 0; c_pos.y < CHUNK_SIZE; ++c_pos.y)
	for(c_pos.z = 0; c_pos.z < CHUNK_SIZE; ++c_pos.z, ++block_i)
	{
		cache[block_i] = chunk.get_block(c_pos).get();
	}

	block_i = 0;
	for(block_in_chunk::value_type x = 0; x < CHUNK_SIZE; ++x)
	for(block_in_chunk::value_type y = 0; y < CHUNK_SIZE; ++y)
	for(block_in_chunk::value_type z = 0; z < CHUNK_SIZE; ++z, ++block_i)
	{
		const block::base& block = *cache[block_i];
		if(block.is_invisible())
		{
			continue;
		}

		for(uint8_t face_i = 0; face_i < 6; ++face_i)
		{
			const Face face = static_cast<Face>(face_i);
			const Side side = to_side(face);
			const auto i = get_i(face);
			glm::tvec3<int8_t> pos(x, y, z);
			pos[i.y] += static_cast<int8_t>(side);

			bool is_visible;
			if(pos.x >= 0 && pos.x < CHUNK_SIZE
			&& pos.y >= 0 && pos.y < CHUNK_SIZE
			&& pos.z >= 0 && pos.z < CHUNK_SIZE)
			{
				const std::size_t sibling_i = static_cast<std::size_t>(CHUNK_SIZE * CHUNK_SIZE * pos.x + CHUNK_SIZE * pos.y + pos.z);
				const block::base& sibling = *cache[sibling_i];
				is_visible =
					   sibling.type() != block::enums::type::none
					&& !block.is_invisible() // this block is visible
					&& !sibling.is_opaque() // this block can be seen thru the adjacent block
					&& block.type() != sibling.type() // do not show sides inside of adjacent translucent blocks of the same type
				;
			}
			else
			{
				is_visible = block_visible_from(chunk, block, pos.x, pos.y, pos.z);
			}
			if(is_visible)
			{
				const auto tex = block.texture_info(face);
				const meshmap_key_t key =
				{
					block.shader_path(face),
					block.is_translucent(),
					tex.unit,
				};
				base::add_face(meshes[key], {x, y, z}, face, 1, 1, tex.index, block.rotation(face));
			}
		}
	}

	return meshes;
}

}
