#include include/header.fs

vec4 color(vec2 uv)
{
	float offset = sin(global_time) + sin(2 * global_time) + sin(3 * global_time);
	float len = length(uv - 0.5);
	float m = 999999;
	float r = (cos(      offset + m * len) + 1) / 2;
	float g = (cos(0.1 + offset + m * len) + 1) / 2;
	float b = (cos(0.2 + offset + m * len) + 1) / 2;
	return vec4(r, g, b, 1);
}

#include include/main.fs
