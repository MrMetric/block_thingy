#include include/header.fs

const float radius_max = 1.0 / 4.0 + 1.0;

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 color(vec2 coords)
{
	float radius = sin(4.0 * global_time + length(position)) / 4.0 + 1.0;
	float radius_scaled = radius / radius_max;
	vec3 c = vec3(1.0 - discretestep(0.3 * radius, length(fract(coords) - 0.5)));
	c *= hsv2rgb(vec3(global_time + radius_scaled, 1.0, 1.0));
	c *= radius_scaled;
	return vec4(c, 1.0);
}

#include include/main.fs
