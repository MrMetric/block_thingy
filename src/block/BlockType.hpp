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
	teleporter,
	crappy_marble,
	white,
	black,
	light_test_red,
	light_test_green,
	light_test_blue,
	light_test_yellow,
	light_test_cyan,
	light_test_pink,
	light_test_white,
	COUNT,
};

#define BlockType_COUNT static_cast<block_type_id_t>(BlockType::COUNT)
