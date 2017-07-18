#version 330

layout(location = 0) in vec3 relative_position;
layout(location = 1) in float face_in;
layout(location = 2) in vec3 light1_in;
layout(location = 3) in vec3 light2_in;
layout(location = 4) in vec3 light3_in;
layout(location = 5) in vec3 light4_in;
layout(location = 6) in float tex_index_in;
layout(location = 7) in float rotation_in;

uniform mat4 mvp_matrix;
uniform vec3 position_offset;

out vec3 position;
flat out int face;
flat out vec3 light1;
flat out vec3 light2;
flat out vec3 light3;
flat out vec3 light4;
flat out int tex_index;
flat out int rotation;

void main()
{
	position = relative_position + position_offset;
	face = int(face_in);
	light1 = light1_in;
	light2 = light2_in;
	light3 = light3_in;
	light4 = light4_in;
	tex_index = int(tex_index_in);
	rotation = int(rotation_in);
	gl_Position = mvp_matrix * vec4(position, 1);
}
