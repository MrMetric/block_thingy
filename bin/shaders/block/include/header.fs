#version 330

const float M_TAU = 6.283185307179586476925286766559;

uniform float global_time;
uniform sampler2DArray tex;

in vec3 position;
flat in int face;
flat in vec3 light1;
flat in vec3 light2;
flat in vec3 light3;
flat in vec3 light4;
flat in int tex_index;

out vec4 FragColor;

float discretestep(float split, float x)
{
	return (sign(x - split) + 1) / 2;
}

const int FACE_RIGHT  = 0; // +x
const int FACE_LEFT   = 1; // -x
const int FACE_TOP    = 2; // +y
const int FACE_BOTTOM = 3; // -y
const int FACE_FRONT  = 4; // +z
const int FACE_BACK   = 5; // -z

vec2 get_face_coords()
{
	if(face == FACE_RIGHT || face == FACE_LEFT) return position.zy;
	if(face == FACE_TOP || face == FACE_BOTTOM) return position.xz;
	if(face == FACE_FRONT || face == FACE_BACK) return position.xy;
	return vec2(0);
}
