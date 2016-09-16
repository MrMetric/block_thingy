#include include/header.fs

vec4 color(vec2 coords)
{
	float offset = sin(global_time) + sin(2.0 * global_time) + sin(3.0 * global_time);
	float len = length(fract(coords) - 0.5);
	float m = 999999.0;
	float r = (cos(      offset + m * len) + 1.0) / 2.0;
	float g = (cos(0.1 + offset + m * len) + 1.0) / 2.0;
	float b = (cos(0.2 + offset + m * len) + 1.0) / 2.0;
	return vec4(r, g, b, 1.0);
}

#include include/main.fs
