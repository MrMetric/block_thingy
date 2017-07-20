#version 330

layout(location = 0) in vec3 relative_position;
layout(location = 1) in float face_and_rotation_in;
layout(location = 2) in vec3 light1_in;
layout(location = 3) in vec3 light2_in;
layout(location = 4) in vec3 light3_in;
layout(location = 5) in vec3 light4_in;
layout(location = 6) in float tex_index_in;

uniform mat4 mvp_matrix;
uniform vec3 position_offset;

out vec3 position;
flat out int face;
flat out int rotation;
flat out vec3 light1;
flat out vec3 light2;
flat out vec3 light3;
flat out vec3 light4;
flat out int tex_index;

void main()
{
	position = relative_position + position_offset;
	int face_and_rotation = int(face_and_rotation_in);
	face = face_and_rotation & 7;
	rotation = face_and_rotation >> 3;
	light1 = light1_in;
	light2 = light2_in;
	light3 = light3_in;
	light4 = light4_in;
	tex_index = int(tex_index_in);
	gl_Position = mvp_matrix * vec4(position, 1);
}
