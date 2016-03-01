#include "BlockInChunk.hpp"

#include <cmath>
#include <sstream>
#include <stdexcept>
#include <string>

#include "BlockInWorld.hpp"
#include "../chunk/Chunk.hpp"

namespace Position
{
	BlockInChunk::BlockInChunk() : x(0), y(0), z(0) {}

	BlockInChunk::BlockInChunk(const BlockInChunk_type x, const BlockInChunk_type y, const BlockInChunk_type z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		check_bounds();
	}

	// % can return a negative result, so do it properly here
	// TODO: find out if std::mod works instead
	#undef t
	#define t(a) static_cast<BlockInChunk_type>(a - CHUNK_SIZE * std::floor(a / static_cast<double>(CHUNK_SIZE)))
	BlockInChunk::BlockInChunk(const BlockInWorld& pos)
	{
		x = t(pos.x);
		y = t(pos.y);
		z = t(pos.z);
	}

	BlockInChunk_type& BlockInChunk::operator[](uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("Position::BlockInChunk::operator[]: " + std::to_string(i) + " > 2");
	}

	BlockInChunk& BlockInChunk::operator+=(const BlockInChunk& that)
	{
		x += that.x;
		y += that.y;
		z += that.z;
		check_bounds();
		return *this;
	}

	void BlockInChunk::check_bounds()
	{
		if(x >= CHUNK_SIZE
		|| y >= CHUNK_SIZE
		|| z >= CHUNK_SIZE)
		{
			std::ostringstream ss;
			ss << "Position::BlockInChunk has invalid coordinates: (" << int(x) << "," << int(y) << "," << int(z) << ")";
			throw std::logic_error(ss.str());
		}
	}

	std::ostream& operator<<(std::ostream& os, const BlockInChunk& pos)
	{
		os << "(" << int(pos.x) << "," << int(pos.y) << "," << int(pos.z) << ")";
		return os;
	}
}
