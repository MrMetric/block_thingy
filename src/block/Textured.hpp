#pragma once
#include "Base.hpp"

#include <array>

#include "util/filesystem.hpp"

namespace block_thingy::block {

class Textured : public Base
{
public:
	Textured(enums::Type, const fs::path& texture         , const fs::path& shader);
	Textured(enums::Type, const fs::path& texture         , const std::array<fs::path, 6>& shaders);
	Textured(enums::Type, std::array<fs::path, 6> textures, const fs::path& shader);
	Textured(enums::Type, std::array<fs::path, 6> textures, const std::array<fs::path, 6>& shaders);
	virtual ~Textured();

	Textured(enums::Type); // temporary; needed due to a design flaw
};

}
