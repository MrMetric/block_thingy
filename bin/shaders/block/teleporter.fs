#include include/header.fs

vec3 color(vec2 coords)
{
	vec2 c = fract(coords);
	vec3 color = vec3(c, length(c));
	c *= 4.0;
	float b = abs(cos(c.x + global_time) * sin(c.x) + cos(c.y + global_time) * sin(c.y)) / 2.0;
	return color * b;
}

void main()
{
	vec2 coords = get_side_coords();
	vec3 c = color(coords);
	float a = (3.9 - pow((1.0 - cos(global_time)) / 2.0, 4.0)) / 4.0;
	a -= 8.0 / cosh(8.0 * (global_time - 1));
	FragColor = vec4(c, sqrt(length(c)));
}
