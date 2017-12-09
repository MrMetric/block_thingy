#pragma once

#include <string>

#include "util/filesystem.hpp"

namespace language {

std::string get(const std::string& key);
std::string get(const std::string& language, const std::string& key);
void set(const std::string& language, const std::string& key, std::string value);

void load(const std::string& language, const fs::path&);
void load_text(const std::string& language, const std::string&);

}
