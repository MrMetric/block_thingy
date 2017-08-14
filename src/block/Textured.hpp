#pragma once
#include "SimpleShader.hpp"

#include <unordered_map>

namespace Block {

class Textured : public SimpleShader
{
public:
	Textured(Enum::Type);
	Textured(Enum::Type, const fs::path& texture_path);
	Textured(Enum::Type, const std::unordered_map<Enum::Face, fs::path> texture_paths);

	Block::Textured& operator=(const Block::Base& block) override;

	Enum::VisibilityType visibility_type() const override
	{
		return visibility_type_;
	}

protected:
	fs::path texture_(Enum::Face) const override;

private:
	std::unordered_map<Enum::Face, fs::path> texture_paths;
	Enum::VisibilityType visibility_type_;
};

}
