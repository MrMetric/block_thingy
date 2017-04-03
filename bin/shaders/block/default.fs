#include include/header.fs

vec4 color(vec2 uv)
{
	if(uv.x <  0.5 && uv.y <  0.5
	|| uv.x >= 0.5 && uv.y >= 0.5)
	{
		return vec4(0, 0, 0, 1);
	}
	return vec4(1, 0, 1, 1);
}

#include include/main.fs
