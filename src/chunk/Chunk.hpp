#pragma once

#include <array>
#include <memory>
#include <stdint.h>
#include <vector>

#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "fwd/World.hpp"
#include "block/Base.hpp"
#include "chunk/mesh/ChunkMesher.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/ChunkInWorld.hpp"

// signed instead of std::size_t to allow comparison with signed values
constexpr int_fast32_t CHUNK_SIZE = 32;
constexpr int_fast32_t CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

using chunk_block_array_t = std::array<std::unique_ptr<Block::Base>, CHUNK_BLOCK_COUNT>;

class Chunk
{
	friend class World;

	public:
		Chunk(const Position::ChunkInWorld&, World& owner);

		Chunk(Chunk&&) = delete;
		Chunk(const Chunk&) = delete;
		void operator=(const Chunk&) = delete;

		World& get_owner() const; // eeh
		Position::ChunkInWorld get_position() const;

		const Block::Base& get_block(Position::BlockInChunk::value_type x, Position::BlockInChunk::value_type y, Position::BlockInChunk::value_type z) const;
		const Block::Base& get_block(const Position::BlockInChunk&) const;

		void set_block(Position::BlockInChunk::value_type x, Position::BlockInChunk::value_type y, Position::BlockInChunk::value_type z, std::unique_ptr<Block::Base>);
		void set_block(const Position::BlockInChunk&, const std::unique_ptr<Block::Base>);

		const Graphics::Color& get_light(const Position::BlockInChunk&) const;
		void set_light(const Position::BlockInChunk&, const Graphics::Color&);

		void update();
		void render(bool transluscent_pass);

		const meshmap_t& get_meshes() const;
		void set_meshes(const meshmap_t&);

		// for loading
		void set_blocks(std::unique_ptr<chunk_block_array_t>);
		void set_blocks(std::unique_ptr<Block::Base>);

		// public because friend stuff does not work for msgpack stuff
		std::unique_ptr<chunk_block_array_t> blocks;
		std::unique_ptr<Block::Base> solid_block;

	private:
		World& owner;
		Position::ChunkInWorld position;
		bool changed;

		std::array<Graphics::Color, CHUNK_BLOCK_COUNT> light;

		meshmap_t meshes;
		std::vector<Graphics::OpenGL::VertexArray> mesh_vaos;
		std::vector<Graphics::OpenGL::VertexBuffer> mesh_vbos;

		void update_vaos();

		void init_block_array();

		void update_neighbors() const;
		void update_neighbors(Position::BlockInChunk::value_type, Position::BlockInChunk::value_type, Position::BlockInChunk::value_type) const;
		void update_neighbor(Position::ChunkInWorld::value_type, Position::ChunkInWorld::value_type, Position::ChunkInWorld::value_type) const;
};
