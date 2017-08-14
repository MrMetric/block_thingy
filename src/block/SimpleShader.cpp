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

SimpleShader& SimpleShader::operator=(const Base& block)
{
	Base::operator=(block);
	const SimpleShader* that = static_cast<const SimpleShader*>(&block);
	shader_path = that->shader_path;
	return *this;
}

fs::path SimpleShader::shader_(const Enum::Face) const
{
	return shader_path;
}

}
