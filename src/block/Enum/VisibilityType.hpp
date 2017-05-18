#pragma once

#include <stdint.h>

namespace Block::Enum {

enum class VisibilityType : uint8_t
{
	opaque,
	translucent,
	invisible,
};

}
