#pragma once
#include "base.hpp"

#include <array>

#include "util/filesystem.hpp"

namespace block_thingy::block {

class textured : public base
{
public:
	textured(enums::type, const fs::path& texture         , const fs::path& shader);
	textured(enums::type, const fs::path& texture         , const std::array<fs::path, 6>& shaders);
	textured(enums::type, std::array<fs::path, 6> textures, const fs::path& shader);
	textured(enums::type, std::array<fs::path, 6> textures, const std::array<fs::path, 6>& shaders);
	virtual ~textured();

	textured(enums::type); // temporary; needed due to a design flaw
};

}
