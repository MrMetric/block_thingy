#version 330

out vec4 FragColor;
in vec3 vPos;

float color(vec2 vec)
{
	return (sign(length(fract(vec) - 0.5) - 0.45) + 1.0) / 2.0;
}

void main()
{
	float pattern1 = color(vPos.xy);
	float pattern2 = color(vPos.xz);
	float pattern3 = color(vPos.yz);
	vec3 vColor = 1.0 - vec3(pattern1, pattern2, pattern3);
	FragColor = vec4(vColor, 1.0);
}
