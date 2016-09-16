#include include/header.fs

vec4 color(vec2 coords)
{
	vec2 p = fract(coords);
	vec3 c = vec3(p, length(p));
	p *= 4.0;
	c *= abs(cos(p.x + global_time) * sin(p.x) + cos(p.y + global_time) * sin(p.y)) / 2.0;
	return vec4(c, sqrt(length(c)));
}

#include include/main.fs
