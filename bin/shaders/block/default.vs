#version 330

layout(location = 0) in vec3 relative_position_in;
layout(location = 1) in float face_and_rotation_in;
layout(location = 2) in float tex_index_in;

uniform mat4 mvp_matrix;
uniform vec3 position_offset;

out vec3 relative_position;
out vec3 position;
flat out int face;
flat out int rotation;
flat out int tex_index;

void main()
{
	relative_position = relative_position_in;
	position = relative_position_in + position_offset;
	int face_and_rotation = int(face_and_rotation_in);
	face = face_and_rotation & 7;
	rotation = face_and_rotation >> 3;
	tex_index = int(tex_index_in);
	gl_Position = mvp_matrix * vec4(position, 1);
}
