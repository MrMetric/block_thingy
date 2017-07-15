#version 330

layout(location = 0) in vec2 position;
uniform mat4 mvp_matrix;

void main()
{
	gl_Position = mvp_matrix * vec4(position, 0, 1);
}
