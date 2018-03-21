#pragma once

#include <optional>
#include <string>

namespace block_thingy::util {

std::optional<std::string> parse_escape_sequence(std::string::const_iterator&, std::string::const_iterator end);
std::optional<std::string> parse_escape_sequence(const char);
std::optional<std::string> parse_escape_sequence(const std::string&);

}
