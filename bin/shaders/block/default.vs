#version 330

layout(location = 0) in vec3 relative_position;
layout(location = 1) in vec3 light_in;
layout(location = 2) in float face_in;

uniform mat4 matriks;
uniform vec3 position_offset;

out vec3 position;
out vec3 light;
flat out int face;

void main()
{
	position = relative_position + position_offset;
	light = light_in;
	face = int(face_in);
	gl_Position = matriks * vec4(position, 1);
}
