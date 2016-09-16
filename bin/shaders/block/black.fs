#include include/header.fs

vec4 color(vec2 coords)
{
	float x = pow(abs(sin(M_TAU / 2.0 * coords.x)), 1.0 / 8.0);
	float y = pow(abs(sin(M_TAU / 2.0 * coords.y)), 1.0 / 8.0);
	vec3 c = vec3(1.0 - x * y);
	return vec4(c, 1.0);
}

#include include/main.fs
