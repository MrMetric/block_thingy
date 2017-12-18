#include "Textured.hpp"

#include "Game.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/VisibilityType.hpp"

using std::string;

namespace block_thingy::block {

using enums::Face;

static enums::visibility_type get_visibility_type(const fs::path& path)
{
	if(!path.empty() && game::instance->resource_manager.texture_has_transparency(path))
	{
		return enums::visibility_type::translucent;
	}
	return enums::visibility_type::opaque;
}

static enums::visibility_type get_visibility_type(const std::array<fs::path, 6>& texture_paths)
{
	for(const fs::path& path : texture_paths)
	{
		if(path.empty())
		{
			continue;
		}
		if(game::instance->resource_manager.texture_has_transparency(path))
		{
			return enums::visibility_type::translucent;
		}
	}
	return enums::visibility_type::opaque;
}

textured::textured(const enums::type t, const fs::path& texture_path, const fs::path& shader)
:
	base(t, get_visibility_type(texture_path), shader)
{
	texture_.fill(texture_path);
}

textured::textured(const enums::type t, const fs::path& texture_path, const std::array<fs::path, 6>& shaders)
:
	base(t, get_visibility_type(texture_path), shaders)
{
	texture_.fill(texture_path);
}

textured::textured(const enums::type t, std::array<fs::path, 6> textures, const fs::path& shader)
:
	base(t, get_visibility_type(textures), shader)
{
	texture_ = std::move(textures);
}

textured::textured(const enums::type t, std::array<fs::path, 6> textures, const std::array<fs::path, 6>& shaders)
:
	base(t, get_visibility_type(textures), shaders)
{
	texture_ = std::move(textures);
}

textured::~textured()
{
}

textured::textured(const enums::type t)
:
	textured(t, fs::path(), "texture")
{
}

}
