#version 330

layout(location = 0) in vec2 position;
uniform mat4 matriks;

void main()
{
	gl_Position = matriks * vec4(position, 0, 1);
}
