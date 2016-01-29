#version 330
const float TAU = 6.28318530718;

layout (location = 0) in vec3 position;
uniform mat4 matriks;
uniform vec3 pos_mod;
out vec4 vColor;

void main()
{
	vec3 color = mod(position, 2.0);
	/*if(color == vec3(0, 0, 0))
	{
		color = vec3(1, 1, 1);
	}*/
	vColor = vec4(color, 1.0);
	gl_Position = matriks * vec4(position + pos_mod, 1.0);
}