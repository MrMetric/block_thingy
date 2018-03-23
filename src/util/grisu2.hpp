#pragma once

#include <iosfwd>
#include <string>

namespace block_thingy::util {

std::string grisu2(double);

std::ostream& os_grisu2(std::ostream& o);
std::ostream& os_nogrisu2(std::ostream& o);

}
