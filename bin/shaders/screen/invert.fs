#include include/header.fs

void main()
{
	vec4 c = texture(tex, UV);
	FragColor = vec4(1 - c.rgb, c.a);
}
