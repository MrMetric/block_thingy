#pragma once

#include <cstdint>

using block_type_id_t = uint16_t;

enum class BlockType : block_type_id_t
{
	none = 0,
	air = 1,
	test = 2,
};
