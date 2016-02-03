#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include <GL/glew.h>

#include "Block.hpp"
#include "Coords.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class World;

class Chunk
{
	public:
		Chunk(Position::ChunkInWorld pos, World* owner);
		Chunk(const Chunk& chunk);
		virtual ~Chunk();

		// I will worry about copying later
		Block get_block(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;
		Block get_block(Position::BlockInChunk bcp) const;

		Block get2(int_fast16_t x, int_fast16_t y, int_fast16_t z) const;
		void set(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, Block block);
		bool block_is_hidden(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z) const;

		void update();
		void render();

	private:
		World* owner;
		Position::ChunkInWorld pos;
		GLuint vbo_v;
		//std::vector<std::pair<GLuint, GLsizei>> sections; // vbo, draw count
		std::array<Block, CHUNK_BLOCK_COUNT> blok;
		std::vector<GLfloat> vertexes;
		uint_fast64_t vertexes_i;
		//GLuint vbo_e;
		GLsizei draw_count;
		bool changed;

		void init();

		void add_vertexes(int x, int y, int z, int offset, std::vector<GLfloat>& vertexes);
		void draw_cube(BlockInChunk_type x, BlockInChunk_type y, BlockInChunk_type z, std::vector<GLfloat>& vertexes);
};