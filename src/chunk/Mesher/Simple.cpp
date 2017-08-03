#include "Simple.hpp"

#include <unordered_map>

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/Enum/Face.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

using Block::Enum::Face;
using Position::BlockInChunk;

namespace Mesher {

meshmap_t Simple::make_mesh(const Chunk& chunk)
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

		for(uint8_t face_i = 0; face_i < 6; ++face_i)
		{
			const Face face = static_cast<Face>(face_i);
			const Side side = to_side(face);
			const auto i = get_i(face);
			glm::tvec3<int8_t> pos(x, y, z);
			pos[i.y] += static_cast<int8_t>(side);
			if(block_visible_from(chunk, block, pos.x, pos.y, pos.z))
			{
				const auto tex = Game::instance->resource_manager.get_block_texture(block.texture(face));
				const meshmap_key_t key =
				{
					block.type(),
					block.is_translucent(),
					tex.unit,
				};
				Base::add_face(meshes[key], {x, y, z}, face, 1, 1, tex.index, block.rotation(face));
			}
		}
	}
	return meshes;
}

}
