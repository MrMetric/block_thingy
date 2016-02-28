#pragma once

#include <array>
#include <memory>
#include <vector>

#include <graphics/OpenGL/VertexBuffer.hpp>

#include "mesh/ChunkMesher.hpp"
#include "../Block.hpp"
#include "../Coords.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class World;

class Chunk
{
	public:
		Chunk(Position::ChunkInWorld, World* owner);
		Chunk(const Chunk&) = delete;

		const Block& get_block_const(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z) const;
		const Block& get_block_const(const Position::BlockInChunk&) const;
		Block& get_block_mutable(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z);
		Block& get_block_mutable(const Position::BlockInChunk&);

		void set_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, const Block&);
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

		void update_neighbors(const BlockInChunk_type, const BlockInChunk_type, const BlockInChunk_type);
		void update_neighbor(const ChunkInWorld_type, const ChunkInWorld_type, const ChunkInWorld_type);
};
