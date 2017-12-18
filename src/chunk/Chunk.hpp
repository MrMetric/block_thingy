#pragma once

#include <memory>

#include "fwd/World.hpp"
#include "fwd/block/Base.hpp"
#include "chunk/ChunkData.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/position/BlockInChunk.hpp"
#include "fwd/position/ChunkInWorld.hpp"
#include "shim/propagate_const.hpp"

namespace block_thingy {

using chunk_blocks_t = ChunkData<std::shared_ptr<block::base>>;

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

	const std::shared_ptr<block::base> get_block(const position::block_in_chunk&) const;
	std::shared_ptr<block::base> get_block(const position::block_in_chunk&);

	void set_block(const position::block_in_chunk&, const std::shared_ptr<block::base>);

	graphics::color get_blocklight(const position::block_in_chunk&) const;
	void set_blocklight(const position::block_in_chunk&, const graphics::color&);
	void set_texbuflight(const glm::ivec3& pos, const graphics::color&);

	void update();
	void render(bool transluscent_pass);

	// for loading
	void set_blocks(chunk_blocks_t);
	void set_blocks(std::shared_ptr<block::base>);

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
