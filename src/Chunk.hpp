#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <GL/glew.h>

#include "Coords.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class Block;
class World;

class Chunk
{
	public:
		Chunk(Position::ChunkInWorld pos, World* owner);
		Chunk(ChunkInWorld_type chunk_x, ChunkInWorld_type chunk_y, ChunkInWorld_type chunk_z, World* owner);
		Chunk(const Chunk& chunk);
		virtual ~Chunk();

		__attribute__((pure))
		std::shared_ptr<Block> get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;

		__attribute__((pure))
		std::shared_ptr<Block> get_block(Position::BlockInChunk bcp) const;

		std::shared_ptr<Block> get2(int_fast16_t x, int_fast16_t y, int_fast16_t z) const;
		void set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, std::shared_ptr<Block> block);
		bool block_is_hidden(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;

		void update();
		void render();

	private:
		World* owner;
		Position::ChunkInWorld pos;
		GLuint vbo_v;
		//std::vector<std::pair<GLuint, GLsizei>> sections; // vbo, draw count
		std::shared_ptr<Block>* blok;
		std::vector<GLfloat> vertexes;
		uint_fast64_t vertexes_i;
		//GLuint vbo_e;
		GLsizei draw_count;
		bool changed;

		void init();

		void add_vertexes(int x, int y, int z, int offset, std::vector<GLfloat>& vertexes);
		void draw_cube(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, std::vector<GLfloat>& vertexes);
};