#pragma once

#include <stdint.h>

using block_type_id_t = uint16_t;

enum class BlockType : block_type_id_t
{
	none,
	air,
	light = 9, // TODO: delete this
};
