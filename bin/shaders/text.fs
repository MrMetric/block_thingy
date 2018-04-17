#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform float faintness;
uniform vec4 color;
uniform sampler2D text;

void main()
{
	float a = texture(text, TexCoords).r;
	vec4 c = vec4(color.rgb, pow(a, faintness + 1));
	FragColor = c;
}
