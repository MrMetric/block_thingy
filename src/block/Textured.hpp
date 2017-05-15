#pragma once
#include "Base.hpp"

namespace Block {

class Textured : public Base
{
public:
	Textured(BlockType);

	fs::path texture(Enum::Face) const override;

	BlockVisibilityType visibility_type() const override;
};

}
