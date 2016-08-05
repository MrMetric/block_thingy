#include include/header.fs

vec3 color(vec2 coords)
{
	vec2 c = fract(coords);
	if(c.x < 0.5 && c.y < 0.5
	|| c.x >= 0.5 && c.y >= 0.5)
	{
		return vec3(0.0, 0.0, 0.0);
	}
	return vec3(1.0, 0.0, 1.0);
}

#include include/main.fs
