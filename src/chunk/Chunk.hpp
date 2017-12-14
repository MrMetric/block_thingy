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

using chunk_blocks_t = ChunkData<std::shared_ptr<block::Base>>;

class Chunk
{
public:
	Chunk(const position::ChunkInWorld&, World& owner);
	~Chunk();

	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	Chunk& operator=(Chunk&&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	World& get_owner() const; // eeh
	position::ChunkInWorld get_position() const;

	const std::shared_ptr<block::Base> get_block(const position::BlockInChunk&) const;
	std::shared_ptr<block::Base> get_block(const position::BlockInChunk&);

	void set_block(const position::BlockInChunk&, const std::shared_ptr<block::Base>);

	graphics::Color get_blocklight(const position::BlockInChunk&) const;
	void set_blocklight(const position::BlockInChunk&, const graphics::Color&);
	void set_texbuflight(const glm::ivec3& pos, const graphics::Color&);

	void update();
	void render(bool transluscent_pass);

	// for loading
	void set_blocks(chunk_blocks_t);
	void set_blocks(std::shared_ptr<block::Base>);

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	friend class World;

	chunk_blocks_t blocks; // this here (instead of in impl) for msgpack saving

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
