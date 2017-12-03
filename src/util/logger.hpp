#pragma once

#include <iostream>
#include <string>

#define LOG(X) ::block_thingy::log(#X)

namespace block_thingy {

std::ostream& log(const std::string& category);

}
