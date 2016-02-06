#pragma once

#include <array>
#include <memory>
#include <vector>

#include <GL/glew.h>

#include "../Block.hpp"
#include "../Coords.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class ChunkMesher;
class World;

class Chunk
{
	public:
		Chunk(Position::ChunkInWorld pos, World* owner);
		Chunk(const Chunk&) = delete;
		virtual ~Chunk();

		// I will worry about copying later
		Block get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;
		Block get_block(Position::BlockInChunk bcp) const;

		void set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, Block block);

		Position::ChunkInWorld get_position() const;
		World* get_owner() const; // eeh

		void update();
		void render();

	private:
		World* owner;
		Position::ChunkInWorld position;
		GLuint mesh_vbo;
		std::unique_ptr<ChunkMesher> mesher;
		std::array<Block, CHUNK_BLOCK_COUNT> blok;
		std::vector<GLbyte> vertexes;
		GLsizei draw_count;
		bool changed;

		void init();

		void update_neighbors(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z);
		void update_neighbor(ChunkInWorld_type x, ChunkInWorld_type y, ChunkInWorld_type z);
};
