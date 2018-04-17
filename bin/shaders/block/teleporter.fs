#include include/header.fs

vec4 color(vec2 uv)
{
	vec3 c = vec3(uv, length(uv));
	vec2 p = uv * 4;
	c *= abs(cos(p.x + world_time) * sin(p.x) + cos(p.y + world_time) * sin(p.y)) / 2;
	return vec4(c, sqrt(length(c)));
}

#include include/main.fs
