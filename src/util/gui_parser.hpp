#pragma once

#include <string>
#include <vector>

#include "util/filesystem.hpp"

#include <json.hpp>

namespace block_thingy::util {

json parse_gui(const fs::path& gui_path);
json parse_gui(const std::string& gui_text);
std::vector<std::string> parse_expression(const std::string&);

}
