#pragma once

#include <algorithm>
#include <array>
#include <memory>

#include "fwd/block/Base.hpp"
#include "position/BlockInChunk.hpp"

#include "fwd/chunk/Chunk.hpp"

template<typename T>
class ChunkData
{
public:
	using chunk_data_t = std::array<T, CHUNK_BLOCK_COUNT>;
	using size_type = typename chunk_data_t::size_type;
	using const_iterator = typename chunk_data_t::const_iterator;
	using iterator = typename chunk_data_t::iterator;

	ChunkData()
	{
	}

	ChunkData(const T& block)
	{
		fill(block);
	}

	ChunkData(ChunkData&& that)
	:
		blocks(std::move(that.blocks))
	{
	}
	void operator=(ChunkData&& that)
	{
		blocks = std::move(that.blocks);
	}

	ChunkData(const ChunkData&) = delete;
	ChunkData& operator=(const ChunkData&) = delete;

	const T& get(const Position::BlockInChunk& pos) const
	{
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	T& get(const Position::BlockInChunk& pos)
	{
		return blocks[block_array_index(pos.x, pos.y, pos.z)];
	}

	void set(const Position::BlockInChunk& pos, T block)
	{
		const auto i = block_array_index(pos.x, pos.y, pos.z);
		blocks[i] = std::move(block);
	}

	void fill(const T& block)
	{
		std::generate(blocks.begin(), blocks.end(), [&block]()
		{
			return block;
		});
	}

	const_iterator cbegin() const
	{
		return blocks.cbegin();
	}
	const_iterator cend() const
	{
		return blocks.cend();
	}
	iterator begin()
	{
		return blocks.begin();
	}
	iterator end()
	{
		return blocks.end();
	}

	// public for msgpack
	chunk_data_t blocks;

private:
	size_type block_array_index
	(
		const Position::BlockInChunk::value_type x,
		const Position::BlockInChunk::value_type y,
		const Position::BlockInChunk::value_type z
	)
	const
	{
		return CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
	}
};

template<>
ChunkData<std::unique_ptr<Block::Base>>::ChunkData();

template<>
void ChunkData<std::unique_ptr<Block::Base>>::fill(const std::unique_ptr<Block::Base>&);
