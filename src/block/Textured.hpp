#pragma once
#include "Base.hpp"

namespace Block {

class Textured : public Base
{
public:
	Textured(Enum::Type);

	Enum::VisibilityType visibility_type() const override;

protected:
	fs::path texture_(Enum::Face) const override;
};

}
