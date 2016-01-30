#pragma once

#include <cstdint>
#include <vector>

#include <GL/glew.h>

#include "Coords.hpp"

#define CHUNK_SIZE 32
#define CHUNK_BLOCK_COUNT (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

class Block;

class Chunk
{
	public:
		Chunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z);
		Chunk(const Chunk& chunk);
		virtual ~Chunk();

		Position::ChunkInWorld pos;

		__attribute__((pure))
		Block* get_block(uint8_t x, uint8_t y, uint8_t z) const;

		__attribute__((pure))
		Block* get_block(Position::BlockInChunk bcp) const;

		Block* get2(int_fast16_t x, int_fast16_t y, int_fast16_t z) const;
		void set(uint8_t x, uint8_t y, uint8_t z, Block* block, bool delete_old_block = true);
		bool block_is_hidden(uint8_t x, uint8_t y, uint8_t z) const;

		void update();
		void render();

	private:
		GLuint vbo_v;
		//std::vector<std::pair<GLuint, GLsizei>> sections; // vbo, draw count
		Block** blok;
		std::vector<GLfloat> vertexes;
		uint_fast64_t vertexes_i;
		//GLuint vbo_e;
		GLsizei draw_count;
		bool changed;

		void init();

		void add_vertexes(int x, int y, int z, int offset, std::vector<GLfloat>& vertexes);
		void draw_cube(uint8_t x, uint8_t y, uint8_t z, std::vector<GLfloat>& vertexes);
};