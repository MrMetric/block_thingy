#pragma once

#include <array>
#include <memory>
#include <vector>

#include <glad/glad.h>

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
		virtual ~Chunk();

		// I will worry about copying later
		Block get_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z) const;
		Block get_block(const Position::BlockInChunk&) const;

		void set_block(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z, Block);
		void set_block(const Position::BlockInChunk&, Block);

		Position::ChunkInWorld get_position() const;
		World* get_owner() const; // eeh

		void update();
		void render();

	private:
		World* owner;
		Position::ChunkInWorld position;
		std::array<Block, CHUNK_BLOCK_COUNT> blok;
		std::unique_ptr<ChunkMesher> mesher;
		mesh_t meshes;
		std::vector<GLuint> mesh_vbos;
		bool changed;

		void update_neighbors(const BlockInChunk_type, const BlockInChunk_type, const BlockInChunk_type);
		void update_neighbor(const ChunkInWorld_type, const ChunkInWorld_type, const ChunkInWorld_type);
};
