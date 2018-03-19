#pragma once

#include <stdint.h>
#include <string>

namespace block_thingy::util {

uint32_t crc32(const std::string&, uint32_t initial_value = 0);

}
