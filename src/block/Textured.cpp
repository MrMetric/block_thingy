#include "Textured.hpp"

#include <array>

#include "Game.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/VisibilityType.hpp"

using std::string;

using Block::Enum::Face;

namespace Block {

static Enum::VisibilityType get_visibility_type(const fs::path& path)
{
	if(!path.empty() && Game::instance->resource_manager.texture_has_transparency(path))
	{
		return Enum::VisibilityType::translucent;
	}
	return Enum::VisibilityType::opaque;
}

static Enum::VisibilityType get_visibility_type(const std::array<fs::path, 6>& texture_paths)
{
	for(const fs::path& path : texture_paths)
	{
		if(path.empty())
		{
			continue;
		}
		if(Game::instance->resource_manager.texture_has_transparency(path))
		{
			return Enum::VisibilityType::translucent;
		}
	}
	return Enum::VisibilityType::opaque;
}

Textured::Textured(const Enum::Type t, const fs::path& texture_path, const fs::path& shader)
:
	Base(t, get_visibility_type(texture_path), shader)
{
	texture_.fill(texture_path);
}

Textured::Textured(const Enum::Type t, const fs::path& texture_path, const std::array<fs::path, 6>& shaders)
:
	Base(t, get_visibility_type(texture_path), shaders)
{
	texture_.fill(texture_path);
}

Textured::Textured(const Enum::Type t, std::array<fs::path, 6> textures, const fs::path& shader)
:
	Base(t, get_visibility_type(textures), shader)
{
	texture_ = std::move(textures);
}

Textured::Textured(const Enum::Type t, std::array<fs::path, 6> textures, const std::array<fs::path, 6>& shaders)
:
	Base(t, get_visibility_type(textures), shaders)
{
	texture_ = std::move(textures);
}

Textured::~Textured()
{
}

Textured::Textured(const Enum::Type t)
:
	Textured(t, fs::path(), "texture")
{
}

}
