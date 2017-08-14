#include "SimpleShader.hpp"

namespace Block {

SimpleShader::SimpleShader(const Enum::Type t)
:
	SimpleShader(t, "default")
{
}

SimpleShader::SimpleShader
(
	const Enum::Type t,
	const fs::path& shader_path
)
:
	Base(t),
	shader_path("shaders/block" / shader_path)
{
}

fs::path SimpleShader::shader_(const Enum::Face) const
{
	return shader_path;
}

}
