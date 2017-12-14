#pragma once

#include <string>

namespace block_thingy::util {

std::u32string utf8_to_utf32(const std::string&);
std::string utf32_to_utf8(const std::u32string&);

}
