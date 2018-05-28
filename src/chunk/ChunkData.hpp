#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <mutex>
#include <type_traits>

#include "game.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/block_in_chunk.hpp"

namespace block_thingy {

template<typename T>
class chunk_data
{
public:
	chunk_data()
	{
	}

	chunk_data(T block)
	{
		fill(std::move(block));
	}

	chunk_data(chunk_data&& that)
	:
		blocks(std::move(that.blocks))
	{
	}
	chunk_data& operator=(chunk_data&& that)
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		blocks = std::move(that.blocks);
		return *this;
	}

	chunk_data(const chunk_data&) = delete;
	chunk_data& operator=(const chunk_data&) = delete;

	const T& get(const position::block_in_chunk& pos) const
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	T& get(const position::block_in_chunk& pos)
	{
		std::lock_guard<std::mutex> g(blocks_mutex);
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	void set(const position::block_in_chunk& pos, T block)
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
		const position::block_in_chunk::value_type x,
		const position::block_in_chunk::value_type y,
		const position::block_in_chunk::value_type z
	)
	{
		return CHUNK_SIZE * CHUNK_SIZE * x + CHUNK_SIZE * y + z;
	}
};

}
