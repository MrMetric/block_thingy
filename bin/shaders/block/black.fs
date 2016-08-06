#include include/header.fs

vec3 color(vec2 coords)
{
	float c1 = pow(sin(M_TAU / 2.0 * coords.x), 1.0/8.0);
	float c2 = pow(sin(M_TAU / 2.0 * coords.y), 1.0/8.0);
	return vec3(1.0 - c1 * c2);
}

#include include/main.fs
