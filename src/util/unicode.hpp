#pragma once

#include <string>

namespace Util {

std::u32string utf8_to_utf32(const std::string&);
std::string utf32_to_utf8(const std::u32string&);

} // namespace Util
