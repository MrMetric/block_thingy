#include <functional>
#include <stdint.h>
#include <unordered_map>

namespace Position {

template<typename T>
uint64_t hasher(const T& pos)
{
	// x has 1 more bit than y/z because there is room for it
	// y/z are both 21 bits
	// 64 - 21*2 = 22
	const uint64_t x = pos.x & 0x3FFFFF;
	const uint64_t y = pos.y & 0x1FFFFF;
	const uint64_t z = pos.z & 0x1FFFFF;
	return	  (x << 42)
			| (y << 21)
			| (z)
		;
}

template<typename P>
using hasher_t = std::function<uint64_t(P)>;

template<typename T>
struct hasher_struct
{
	uint64_t operator()(const T& pos) const
	{
		return hasher(pos);
	}
};

template<typename P, typename T>
using unordered_map_t = std::unordered_map<P, T, hasher_struct<P>>;

}
