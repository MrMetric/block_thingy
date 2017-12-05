#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <mutex>
#include <type_traits>

#include "Game.hpp"
#include "block/Base.hpp"
#include "block/Enum/Type.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

template<typename T>
class ChunkData
{
public:
	ChunkData()
	{
		// explicit template instantiation does not work with both MSVC and GCC/Clang
		// with MSVC, putting a prototype in this file with the definition in a cpp file causes a linking error
		// with GCC/Clang, putting the definition in this file breaks the one-definition rule
		if constexpr(std::is_same<T, std::shared_ptr<Block::Base>>::value)
		{
			fill(Game::instance->block_registry.get_default(Block::Enum::Type::air));
		}
	}

	ChunkData(T block)
	{
		fill(std::move(block));
	}

	ChunkData(ChunkData&& that)
	:
		blocks(std::move(that.blocks))
	{
	}
	ChunkData& operator=(ChunkData&& that)
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		blocks = std::move(that.blocks);
		return *this;
	}

	ChunkData(const ChunkData&) = delete;
	ChunkData& operator=(const ChunkData&) = delete;

	const T& get(const Position::BlockInChunk& pos) const
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	T& get(const Position::BlockInChunk& pos)
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	void set(const Position::BlockInChunk& pos, T block)
	{
		const std::size_t i = block_array_index(pos.x, pos.y, pos.z);
		std::lock_guard<std::mutex> g(blocks_mutex);
		blocks[i] = std::move(block);
	}

	void fill(T block)
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		std::generate(blocks.begin(), blocks.end(), [&block]()
		{
			return block;
		});
	}

	// for msgpack
	template<typename O> void save(O&) const;
	template<typename O> void load(const O&);

private:
	std::array<T, CHUNK_BLOCK_COUNT> blocks;
	mutable std::mutex blocks_mutex;

	static std::size_t block_array_index
	(
		const Position::BlockInChunk::value_type x,
		const Position::BlockInChunk::value_type y,
		const Position::BlockInChunk::value_type z
	)
	{
		return CHUNK_SIZE * CHUNK_SIZE * x + CHUNK_SIZE * y + z;
	}
};
