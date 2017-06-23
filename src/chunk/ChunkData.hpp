#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <mutex>

#include "fwd/block/Base.hpp"
#include "fwd/chunk/Chunk.hpp"
#include "position/BlockInChunk.hpp"

template<typename T>
class ChunkData
{
public:
	using chunk_data_t = std::array<T, CHUNK_BLOCK_COUNT>;
	using size_type = typename chunk_data_t::size_type;

	ChunkData()
	{
	}

	ChunkData(T block)
	{
		fill(std::move(block));
	}

	ChunkData(ChunkData&& that)
	:
		solid_block(std::move(that.solid_block)),
		blocks(std::move(that.blocks))
	{
	}
	ChunkData& operator=(ChunkData&& that)
	{
		std::lock_guard<std::mutex> g(mutex);
		solid_block = std::move(that.solid_block);
		blocks = std::move(that.blocks);
		return *this;
	}

	ChunkData(const ChunkData&) = delete;
	ChunkData& operator=(const ChunkData&) = delete;

	const T& get(const Position::BlockInChunk& pos) const
	{
		std::lock_guard<std::mutex> g(mutex);
		if(blocks == nullptr)
		{
			return solid_block;
		}
		return (*blocks)[block_array_index(pos.x, pos.y, pos.z)];
	}

	T& get(const Position::BlockInChunk& pos)
	{
		std::lock_guard<std::mutex> g(mutex);
		if(blocks == nullptr)
		{
			fill_();
		}
		return (*blocks)[block_array_index(pos.x, pos.y, pos.z)];
	}

	void set(const Position::BlockInChunk& pos, T block)
	{
		std::lock_guard<std::mutex> g(mutex);
		if(blocks == nullptr)
		{
			if(T_equal(solid_block, block))
			{
				return;
			}
			fill_();
		}
		const auto i = block_array_index(pos.x, pos.y, pos.z);
		(*blocks)[i] = std::move(block);
	}

	void fill(T block)
	{
		std::lock_guard<std::mutex> g(mutex);
		solid_block = std::move(block);
		blocks = nullptr;
	}

	// for msgpack
	template<typename O> void save(O&) const;
	template<typename O> void load(const O&);

private:
	T solid_block;
	std::unique_ptr<chunk_data_t> blocks;
	mutable std::mutex mutex;

	void fill_()
	{
		blocks = std::make_unique<chunk_data_t>();
		std::generate(blocks->begin(), blocks->end(), [this]()
		{
			return T_copy(solid_block);
		});
	}

	T T_copy(const T& block) const
	{
		return block;
	}
	bool T_equal(const T& a, const T& b) const
	{
		return a == b;
	}

	static size_type block_array_index
	(
		const Position::BlockInChunk::value_type x,
		const Position::BlockInChunk::value_type y,
		const Position::BlockInChunk::value_type z
	)
	{
		return CHUNK_SIZE * CHUNK_SIZE * y + CHUNK_SIZE * z + x;
	}
};

template<>
ChunkData<std::unique_ptr<Block::Base>>::ChunkData();

template<>
std::unique_ptr<Block::Base> ChunkData<std::unique_ptr<Block::Base>>::T_copy(const std::unique_ptr<Block::Base>&) const;

template<>
bool ChunkData<std::unique_ptr<Block::Base>>::T_equal(const std::unique_ptr<Block::Base>&, const std::unique_ptr<Block::Base>&) const;
