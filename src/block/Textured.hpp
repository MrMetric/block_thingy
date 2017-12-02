#pragma once
#include "Base.hpp"

#include <array>
#include <string>

namespace Block {

class Textured : public Base
{
public:
	Textured(Enum::Type, const fs::path& texture         , const fs::path& shader);
	Textured(Enum::Type, const fs::path& texture         , const std::array<fs::path, 6>& shaders);
	Textured(Enum::Type, std::array<fs::path, 6> textures, const fs::path& shader);
	Textured(Enum::Type, std::array<fs::path, 6> textures, const std::array<fs::path, 6>& shaders);
	virtual ~Textured();

	Textured(Enum::Type); // temporary; needed due to a design flaw
};

}
