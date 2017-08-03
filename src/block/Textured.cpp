#include "Textured.hpp"

#include <array>

#include "Game.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/VisibilityType.hpp"

using Block::Enum::Face;

namespace Block {

Textured::Textured(const Enum::Type type)
:
	Base(type)
{
}

Enum::VisibilityType Textured::visibility_type() const
{
	for(uint8_t face_i = 0; face_i < 6; ++face_i)
	{
		const fs::path path = texture(static_cast<Face>(face_i));
		if(Game::instance->resource_manager.texture_has_transparency(path))
		{
			return Enum::VisibilityType::translucent;
		}
	}
	return Enum::VisibilityType::opaque;
}

fs::path Textured::texture_(const Face face) const
{
	return Game::instance->block_registry.texture(type(), face);
}

}
