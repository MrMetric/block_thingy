#version 330

out vec4 FragColor;
in vec3 position;

vec3 color(vec3 coords)
{
	return abs(mod(coords - 1.0, 2.0) - 1.0);
}

void main()
{
	vec3 c = color(position);
	FragColor = vec4(c, 1.0);
}
