#pragma once

#include <string>

namespace Util::Clipboard {

std::string get_text();
void set_text(const std::string&);
void set_text(const std::u32string&);

}
