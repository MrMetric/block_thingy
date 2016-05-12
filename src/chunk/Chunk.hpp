#pragma once

#include <array>
#include <memory>
#include <vector>

#include "graphics/OpenGL/VertexBuffer.hpp"

#include "block/Block.hpp"
#include "chunk/mesh/ChunkMesher.hpp"
#include "position/BlockInChunk.hpp"
#include "position/ChunkInWorld.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class World;

using chunk_block_array_t = std::array<Block::Block, CHUNK_BLOCK_COUNT>;

class Chunk
{
	public:
		Chunk(const Position::ChunkInWorld&, World& owner);

		Chunk(Chunk&&) = delete;
		Chunk(const Chunk&) = delete;
		void operator=(const Chunk&) = delete;

		const Block::Block& get_block_const(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;
		const Block::Block& get_block_const(const Position::BlockInChunk&) const;
		Block::Block& get_block_mutable(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		Block::Block& get_block_mutable(const Position::BlockInChunk&);

		void set_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, const Block::Block&);
		void set_block(const Position::BlockInChunk&, const Block::Block&);

		Position::ChunkInWorld get_position() const;
		World& get_owner() const; // eeh

		void update();
		void render(bool transluscent_pass);

		// public because friend stuff does not work for msgpack stuff
		std::unique_ptr<chunk_block_array_t> blocks;
		Block::Block solid_block;

	private:
		World& owner;
		Position::ChunkInWorld position;
		meshmap_t meshes;
		std::vector<VertexBuffer> mesh_vbos;
		bool changed;

		void init_block_array();

		void update_neighbors(BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
		void update_neighbor(ChunkInWorld_type, ChunkInWorld_type, ChunkInWorld_type);
};
