#pragma once

#include <functional>
#include <stdint.h>

namespace block_thingy {

struct block_t
{
	uint32_t index: 24, generation: 8;

	block_t()
	:
		block_t(0, 0)
	{
	}

	block_t(const uint32_t index, const uint8_t generation)
	:
		index(index),
		generation(generation)
	{
	}

	bool operator==(const block_t that) const
	{
		// I wonder if this is optimized
		return index == that.index
		    && generation == that.generation;
	}
	bool operator!=(const block_t that) const
	{
		return index != that.index
		    || generation != that.generation;
	}

	// to allow use as the key type of std::map
	bool operator<(const block_t that) const
	{
		if(index == that.index)
		{
			return generation < that.generation;
		}
		return index < that.index;
	}
};

}

namespace std
{
	template<>
	struct hash<block_thingy::block_t>
	{
		std::size_t operator()(const block_thingy::block_t block) const
		{
			static_assert(sizeof(block_thingy::block_t) == 4);
			return *reinterpret_cast<const uint32_t*>(&block);
		}
	};
}
