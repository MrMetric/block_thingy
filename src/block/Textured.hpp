#pragma once
#include "Base.hpp"

namespace Block {

class Textured : public Base
{
public:
	Textured(Enum::Type);

	fs::path texture(Enum::Face) const override;

	Enum::VisibilityType visibility_type() const override;
};

}
