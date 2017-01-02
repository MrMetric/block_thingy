#version 330

layout(location = 0) in vec4 relative_position;
uniform mat4 matriks;
uniform vec3 position_offset;
out vec3 position;
flat out int face;

void main()
{
	position = relative_position.xyz + position_offset;
	face = int(relative_position.w);
	gl_Position = matriks * vec4(position, 1.0);
}
