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

const int FACE_FRONT = 0;
const int FACE_BACK = 1;
const int FACE_TOP = 2;
const int FACE_BOTTOM = 3;
const int FACE_RIGHT = 4;
const int FACE_LEFT = 5;

vec2 get_face_coords()
{
	if(face == FACE_FRONT || face == FACE_BACK) return position.xy;
	if(face == FACE_TOP || face == FACE_BOTTOM) return position.xz;
	if(face == FACE_RIGHT || face == FACE_LEFT) return position.zy;
	return vec2(0);
}
