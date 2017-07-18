#include "Textured.hpp"

#include <array>

#include "Game.hpp"
#include "block/RotationUtil.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/VisibilityType.hpp"

using Block::Enum::Face;

namespace Block {

Textured::Textured(const Enum::Type type)
:
	Base(type)
{
}

fs::path Textured::texture(const Face face) const
{
	return Game::instance->block_registry.texture(type(), RotationUtil::rotate_face(face, rotation_));
}

Enum::VisibilityType Textured::visibility_type() const
{
	std::array<fs::path, 6> texture_paths =
	{
		texture(Face::right),
		texture(Face::left),
		texture(Face::top),
		texture(Face::bottom),
		texture(Face::front),
		texture(Face::back),
	};
	for(const fs::path& path : texture_paths)
	{
		if(Game::instance->resource_manager.texture_has_transparency(path))
		{
			return Enum::VisibilityType::translucent;
		}
	}
	return Enum::VisibilityType::opaque;
}

}
