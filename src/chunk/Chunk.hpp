#pragma once

#include <memory>

#include "block/block.hpp"
#include "chunk/ChunkData.hpp"
#include "fwd/graphics/color.hpp"
#include "fwd/position/block_in_chunk.hpp"
#include "fwd/position/chunk_in_world.hpp"
#include "shim/propagate_const.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

using chunk_blocks_t = chunk_data<block_t>;

class Chunk
{
public:
	Chunk(const position::chunk_in_world&, world::world& owner);
	~Chunk();

	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	world::world& get_owner() const; // eeh
	position::chunk_in_world get_position() const;

	block_t get_block(const position::block_in_chunk&) const;
	void set_block(const position::block_in_chunk&, block_t);

	graphics::color get_light(const position::block_in_chunk&) const;

	graphics::color get_blocklight(const position::block_in_chunk&) const;
	void set_blocklight(const position::block_in_chunk&, const graphics::color&);

	graphics::color get_skylight(const position::block_in_chunk&) const;
	void set_skylight(const position::block_in_chunk&, const graphics::color&);

	void set_texbuflight(const glm::ivec3& pos, const graphics::color&);

	void update();
	void render(bool transluscent_pass);

	// for loading
	void set_blocks(chunk_blocks_t);
	void set_blocks(block_t);

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	friend class world::world;

	chunk_blocks_t blocks; // this here (instead of in impl) for msgpack saving

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
