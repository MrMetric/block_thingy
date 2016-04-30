#pragma once

#include <stdint.h>

using block_type_id_t = uint16_t;

enum class BlockType : block_type_id_t
{
	none,
	air,
	test,
	dots,
	eye,
	COUNT,
};

#define BlockType_COUNT static_cast<block_type_id_t>(BlockType::COUNT)
