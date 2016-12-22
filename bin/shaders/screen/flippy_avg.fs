#include include/header.fs

void main()
{
	vec4 c1 = texture(tex, UV);
	vec4 c2 = texture(tex, vec2(1 - UV.x, UV.y));
	vec4 c3 = texture(tex, vec2(UV.x, 1 - UV.y));
	vec4 c4 = texture(tex, vec2(1 - UV.x, 1 - UV.y));
	FragColor = (c1 + c2 + c3 + c4) / 4;
}