#version 330

layout(location = 0) in vec3 position;
uniform mat4 matriks;
uniform vec3 pos_mod;
out vec3 vPos;

void main()
{
	vPos = position;
	gl_Position = matriks * vec4(position + pos_mod, 1.0);
}
