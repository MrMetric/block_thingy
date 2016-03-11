#pragma once

#include <array>
#include <memory>
#include <vector>

#include <graphics/OpenGL/VertexBuffer.hpp>

#include "mesh/ChunkMesher.hpp"
#include "../Block.hpp"
#include "../position/BlockInChunk.hpp"
#include "../position/ChunkInWorld.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class World;

class Chunk
{
	public:
		Chunk(const Position::ChunkInWorld&, World* owner);
		Chunk(const Chunk&) = delete;

		const Block& get_block_const(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;
		const Block& get_block_const(const Position::BlockInChunk&) const;
		Block& get_block_mutable(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		Block& get_block_mutable(const Position::BlockInChunk&);

		void set_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, const Block&);
		void set_block(const Position::BlockInChunk&, const Block&);

		Position::ChunkInWorld get_position() const;
		World* get_owner() const; // eeh

		void update();
		void render();

	private:
		World* owner;
		Position::ChunkInWorld position;
		std::array<Block, CHUNK_BLOCK_COUNT> blok;
		std::unique_ptr<ChunkMesher> mesher;
		meshmap_t meshes;
		std::vector<VertexBuffer> mesh_vbos;
		bool changed;

		void update_neighbors(BlockInChunk_type, BlockInChunk_type, BlockInChunk_type);
		void update_neighbor(ChunkInWorld_type, ChunkInWorld_type, ChunkInWorld_type);
};
