#include include/header.fs

const vec2 offset_r = vec2(-4,  0);
const vec2 offset_g = vec2( 0, -4);
const vec2 offset_b = vec2( 2,  2);

void main()
{
	FragColor = vec4(
		texture(tex, UV + offset_r / tex_size).r,
		texture(tex, UV + offset_g / tex_size).g,
		texture(tex, UV + offset_b / tex_size).b,
		texture(tex, UV).a
	);
}
