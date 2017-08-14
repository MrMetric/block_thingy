#include "Textured.hpp"

#include <array>

#include "Game.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/VisibilityType.hpp"

using Block::Enum::Face;

namespace Block {

static Enum::VisibilityType get_visibility_type(Textured& self)
{
	for(uint8_t face_i = 0; face_i < 6; ++face_i)
	{
		const fs::path path = self.texture(static_cast<Face>(face_i));
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

Textured::Textured(const Enum::Type t)
:
	Textured(t, fs::path())
{
}

Textured::Textured(const Enum::Type t, const fs::path& texture_path)
:
	Textured(t,
	{
		{Face::right , texture_path},
		{Face::left  , texture_path},
		{Face::top   , texture_path},
		{Face::bottom, texture_path},
		{Face::front , texture_path},
		{Face::back  , texture_path},
	})
{
}

Textured::Textured(const Enum::Type t, const std::unordered_map<Face, fs::path> texture_paths)
:
	SimpleShader(t, "texture"),
	texture_paths(std::move(texture_paths)),
	visibility_type_(get_visibility_type(*this))
{
}

Textured& Textured::operator=(const Base& block)
{
	Base::operator=(block);
	const Textured* that = static_cast<const Textured*>(&block);
	texture_paths = that->texture_paths;
	visibility_type_ = that->visibility_type_;
	return *this;
}

fs::path Textured::texture_(const Face face) const
{
	const auto i = texture_paths.find(face);
	if(i == texture_paths.cend())
	{
		return {};
	}
	return i->second;
}

}
