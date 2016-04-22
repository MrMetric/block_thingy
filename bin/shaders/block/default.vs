#version 330

layout(location = 0) in vec3 relative_position;
uniform mat4 matriks;
uniform vec3 position_offset;
out vec3 position;

void main()
{
	position = relative_position + position_offset;
	gl_Position = matriks * vec4(position, 1.0);
}
