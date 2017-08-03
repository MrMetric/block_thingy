#pragma once

#include <memory>
#include <experimental/propagate_const>

#include "fwd/World.hpp"
#include "fwd/block/Base.hpp"
#include "chunk/ChunkData.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/position/BlockInChunk.hpp"
#include "fwd/position/ChunkInWorld.hpp"

using chunk_blocks_t = ChunkData<std::unique_ptr<Block::Base>>;

class Chunk
{
public:
	Chunk(const Position::ChunkInWorld&, World& owner);
	~Chunk();

	Chunk(Chunk&&) = delete;
	Chunk(const Chunk&) = delete;
	void operator=(const Chunk&) = delete;

	World& get_owner() const; // eeh
	Position::ChunkInWorld get_position() const;

	const Block::Base& get_block(const Position::BlockInChunk&) const;
	Block::Base& get_block(const Position::BlockInChunk&);

	void set_block(const Position::BlockInChunk&, const std::unique_ptr<Block::Base>);

	Graphics::Color get_light(const Position::BlockInChunk&) const;
	void set_light(const Position::BlockInChunk&, const Graphics::Color&);
	void set_neighbor_light(const glm::ivec3&, const Graphics::Color&);

	void update();
	void render(bool transluscent_pass);

	// for loading
	void set_blocks(chunk_blocks_t);
	void set_blocks(std::unique_ptr<Block::Base>);


	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	friend class World;

	chunk_blocks_t blocks; // this here (instead of in impl) for msgpack saving

	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
