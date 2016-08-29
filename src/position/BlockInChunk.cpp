#include "BlockInChunk.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <glm/common.hpp>

#include "BlockInWorld.hpp"

#include "chunk/Chunk.hpp"

using std::to_string;

namespace Position
{
	BlockInChunk::BlockInChunk() : x(0), y(0), z(0) {}

	BlockInChunk::BlockInChunk(const value_type x, const value_type y, const value_type z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		check_bounds();
	}

	/*
	+5 % +2 == std::fmod(+5, +2) == 1
	+5 % -2 == std::fmod(+5, -2) == 1
	-5 % +2 == std::fmod(-5, +2) == -1
	-5 % -2 == std::fmod(-5, -2) == -1
	glm::mod(+5.0, +2.0) == 1
	glm::mod(+5.0, -2.0) == -1
	glm::mod(-5.0, +2.0) == 1
	glm::mod(-5.0, -2.0) == -1
	*/
	#undef t
	#define t(a) static_cast<value_type>(glm::mod(static_cast<double>(a), static_cast<double>(CHUNK_SIZE)))
	BlockInChunk::BlockInChunk(const BlockInWorld& pos)
	{
		x = t(pos.x);
		y = t(pos.y);
		z = t(pos.z);
	}

	BlockInChunk::value_type BlockInChunk::operator[](const uint_fast8_t i) const
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("Position::BlockInChunk::operator[]: " + to_string(i) + " > 2");
	}

	BlockInChunk::value_type& BlockInChunk::operator[](const uint_fast8_t i)
	{
		if(i == 0) return x;
		if(i == 1) return y;
		if(i == 2) return z;
		throw std::out_of_range("Position::BlockInChunk::operator[]: " + to_string(i) + " > 2");
	}

	BlockInChunk& BlockInChunk::operator+=(const BlockInChunk& that)
	{
		x += that.x;
		y += that.y;
		z += that.z;
		check_bounds();
		return *this;
	}

	bool BlockInChunk::operator==(const BlockInChunk& that) const
	{
		return (x == that.x) && (y == that.y) && (z == that.z);
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
		return os << "(" << int(pos.x) << "," << int(pos.y) << "," << int(pos.z) << ")";
	}
}
