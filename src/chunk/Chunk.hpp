#pragma once

#include <array>
#include <memory>
#include <vector>

#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

#include "fwd/World.hpp"
#include "block/Base.hpp"
#include "chunk/ChunkData.hpp"
#include "chunk/mesh/Base.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInChunk.hpp"
#include "position/ChunkInWorld.hpp"

using chunk_blocks_t = ChunkData<std::unique_ptr<Block::Base>>;

class Chunk
{
	friend class World;
	friend class Mesher::Base;

	public:
		Chunk(const Position::ChunkInWorld&, World& owner);

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

		void update();
		void render(bool transluscent_pass);

		// for loading
		void set_blocks(chunk_blocks_t);
		void set_blocks(std::unique_ptr<Block::Base>);

		// for msgpack
		template<typename T> void save(T&) const;
		template<typename T> void load(const T&);

	private:
		World& owner;
		Position::ChunkInWorld position;

		chunk_blocks_t blocks;
		std::unique_ptr<Block::Base> solid_block;

		ChunkData<Graphics::Color> light;

		bool changed;
		Mesher::meshmap_t meshes;
		std::vector<Graphics::OpenGL::VertexArray> mesh_vaos;
		std::vector<Graphics::OpenGL::VertexBuffer> mesh_vbos;

		void update_vaos();
};
