#include include/header.fs

const float d = 0.2;
const float r = 0.1;
//const float ring_size = 0.05;
vec4 color(vec2 coords)
{
	vec2 m = fract(coords) - 0.5;
	vec2 o1 = d * vec2(cos(global_time), sin(global_time));
	vec2 o2 = d * vec2(cos(global_time + M_TAU / 4.0), sin(global_time + M_TAU / 4.0));
	if(length(m + o1) < r
	|| length(m - o1) < r
	|| length(m + o2) < r
	|| length(m - o2) < r)
	{
		return vec4(1.0);
	}
	/*
	if(length(m) < r + (d + ring_size) / 2.0
	&& length(m) > r + (d - ring_size) / 2.0)
	{
		return vec3(cos((atan(m.y, m.x) - 4.0 * global_time)) / 2.0 + 0.5);
	}
	*/

	float x = pow(abs(sin(M_TAU / 2.0 * coords.x)), 1.0 / 8.0);
	float y = pow(abs(sin(M_TAU / 2.0 * coords.y)), 1.0 / 8.0);
	vec3 c = vec3(1.0 - x * y);
	return vec4(c, 1.0);
}

#include include/main.fs
