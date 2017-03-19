#include include/header.fs

vec3 color(vec3 coords)
{
	return abs(mod(coords - 1, 2) - 1);
}

void main()
{
	vec3 c = color(position);
	FragColor = vec4(c, 1);
}
