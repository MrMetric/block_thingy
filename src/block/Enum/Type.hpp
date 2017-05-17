#pragma once

#include <stdint.h>

using block_type_id_t = uint16_t;

enum class BlockType : block_type_id_t
{
	none,
	air,
	unknown,
};

enum class BlockTypeExternal : block_type_id_t
{
};
