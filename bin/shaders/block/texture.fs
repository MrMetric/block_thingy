#include include/header.fs

vec4 color(vec2 uv)
{
	return texture(tex, vec3(uv, tex_index));
}

#include include/main.fs
