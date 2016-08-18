#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 color;
uniform sampler2D text;

void main()
{
	float a = texture(text, TexCoords).r;
	vec4 c = vec4(color, a);
	c = pow(c, vec4(1.0 / 2.2)); // gamma correction
	FragColor = c;
}
