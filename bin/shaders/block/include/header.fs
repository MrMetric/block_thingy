#version 330

const float M_TAU = 6.283185307179586476925286766559;

uniform float global_time;
uniform float world_time;
uniform sampler2DArray tex;
uniform sampler3D light;
uniform int light_smoothing;
uniform float min_light;

in vec3 relative_position;
in vec3 position;
flat in int face;
flat in int rotation;
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

vec2 get_face_coords(vec3 position)
{
	if(face == FACE_RIGHT || face == FACE_LEFT) return position.zy;
	if(face == FACE_TOP || face == FACE_BOTTOM) return position.xz;
	if(face == FACE_FRONT || face == FACE_BACK) return position.xy;
	return vec2(0);
}

float linear_to_srgb(float x)
{
	if(x > 0.0031308)
	{
		return 1.055 * pow(x, 1/2.4) - 0.055;
	}
	return 12.92 * x;
}
vec3 linear_to_srgb(vec3 v)
{
	return vec3(linear_to_srgb(v.r), linear_to_srgb(v.g), linear_to_srgb(v.b));
}
vec4 linear_to_srgb(vec4 v)
{
	return vec4(linear_to_srgb(v.rgb), v.a);
}

float srgb_to_linear(float x)
{
	if(x <= 0.04045)
	{
		return x / 12.92;
	}
	return pow((x + 0.055)/1.055, 2.4);
}
vec3 srgb_to_linear(vec3 v)
{
	return vec3(srgb_to_linear(v.r), srgb_to_linear(v.g), srgb_to_linear(v.b));
}
vec4 srgb_to_linear(vec4 v)
{
	return vec4(srgb_to_linear(v.rgb), v.a);
}
