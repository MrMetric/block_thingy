#include include/header.fs

vec4 color(vec2 uv)
{
	const float d = 0.2;
	const float r = 0.1;

	vec2 m = uv - 0.5;
	vec2 o1 = d * vec2(cos(global_time), sin(global_time));
	vec2 o2 = d * vec2(cos(global_time + M_TAU / 4), sin(global_time + M_TAU / 4));
	if(length(m + o1) < r
	|| length(m - o1) < r
	|| length(m + o2) < r
	|| length(m - o2) < r)
	{
		return vec4(1);
	}

	float x = pow(abs(sin(M_TAU / 2 * uv.x)), 1.0 / 8.0);
	float y = pow(abs(sin(M_TAU / 2 * uv.y)), 1.0 / 8.0);
	vec3 c = vec3(srgb_to_linear(1 - x * y));
	return vec4(c, 1);
}

#define NO_FANCY_LIGHT_SMOOTHING
#include include/main.fs
