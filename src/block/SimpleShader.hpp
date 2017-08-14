#pragma once
#include "Base.hpp"

namespace Block {

class SimpleShader : public Base
{
public:
	SimpleShader(Enum::Type);
	SimpleShader(Enum::Type, const fs::path& shader_path);

protected:
	fs::path shader_(Enum::Face) const override;

private:
	fs::path shader_path;
};

}
