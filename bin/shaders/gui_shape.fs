#version 330

uniform vec4 color;
out vec4 FragColor;

void main()
{
	vec3 c = pow(color.rgb, vec3(1.0 / 2.2)); // gamma correction
	FragColor = vec4(c, color.a);
}
