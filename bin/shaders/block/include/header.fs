#version 330

const float M_TAU = 6.283185307179586476925286766559;

in vec3 position;
flat in int face;
out vec4 FragColor;
uniform float global_time;

float discretestep(float split, float x)
{
	return (sign(x - split) + 1) / 2;
}

const int FACE_BACK = 0;
const int FACE_FRONT = 1;
const int FACE_TOP = 2;
const int FACE_BOTTOM = 3;
const int FACE_LEFT = 4;
const int FACE_RIGHT = 5;

vec2 get_face_coords()
{
	if(face == FACE_BACK || face == FACE_FRONT) return position.xy;
	if(face == FACE_TOP || face == FACE_BOTTOM) return position.xz;
	if(face == FACE_LEFT || face == FACE_RIGHT) return position.zy;
	return vec2(0);
}
