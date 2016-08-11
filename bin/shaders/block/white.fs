#include include/header.fs

vec3 color(vec2 coords)
{
	float cx = pow(abs(sin(M_TAU / 2.0 * coords.x)), 1.0/8.0);
	float cy = pow(abs(sin(M_TAU / 2.0 * coords.y)), 1.0/8.0);
	return vec3(cx * cy);
}

#include include/main.fs
