#include include/header.fs

vec4 color(vec2 coords)
{
	vec2 p = fract(coords);
	if(p.x < 0.5 && p.y < 0.5
	|| p.x >= 0.5 && p.y >= 0.5)
	{
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
	return vec4(1.0, 0.0, 1.0, 1.0);
}

#include include/main.fs
