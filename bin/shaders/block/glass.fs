#include include/header.fs

vec4 color(vec2 uv)
{
	float x = pow(abs(sin(M_TAU / 2 * uv.x)), 1.0 / 6.0);
	float y = pow(abs(sin(M_TAU / 2 * uv.y)), 1.0 / 6.0);
	vec3 c = vec3(srgb_to_linear(1 - x * y));
	return vec4(c, length(c));
}

#include include/main.fs
