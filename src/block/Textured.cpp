#include "Textured.hpp"

#include <array>

#include "Game.hpp"
#include "block/BlockVisibilityType.hpp"
#include "block/Enum/Face.hpp"

using Block::Enum::Face;

namespace Block {

Textured::Textured(const BlockType type)
:
	Base(type)
{
}

fs::path Textured::texture(Face face) const
{
	return Game::instance->block_registry.texture(type(), face);
}

BlockVisibilityType Textured::visibility_type() const
{
	std::array<fs::path, 6> texture_paths =
	{
		texture(Face::front),
		texture(Face::back),
		texture(Face::top),
		texture(Face::bottom),
		texture(Face::right),
		texture(Face::left),
	};
	for(const fs::path& path : texture_paths)
	{
		if(Game::instance->resource_manager.texture_has_transparency(path))
		{
			return BlockVisibilityType::translucent;
		}
	}
	return BlockVisibilityType::opaque;
}

}
