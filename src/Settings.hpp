#pragma once

#include <stdint.h>
#include <string>

#include <strict_variant/variant.hpp>

namespace block_thingy::settings {

using value_t = strict_variant::variant<bool, double, int64_t, std::string>;

template<typename T>
bool has(const std::string& name);

template<typename T>
T get(const std::string& name);

template<typename T>
void set(const std::string& name, T);

bool has(const std::string& name);
value_t get(const std::string& name);
void set(const std::string& name, value_t);

void load();
void save();

// for private use; do not call
void add_command_handlers();

}
