#version 330

out vec4 FragColor;
in vec3 vPos;

void main()
{
	vec3 vColor = abs(mod(vPos - 1.0, 2.0) - 1.0);
	FragColor = vec4(vColor, 1.0);
}