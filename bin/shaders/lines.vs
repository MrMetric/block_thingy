#version 330

layout (location = 0) in vec3 position;
uniform mat4 matriks;

void main()
{
	gl_Position = matriks * vec4(position, 1.0);
}