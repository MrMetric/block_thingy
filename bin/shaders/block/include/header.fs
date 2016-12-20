#version 330

const float M_TAU = 6.283185307179586476925286766559;

in vec3 position;
out vec4 FragColor;
uniform float global_time;

float discretestep(float split, float x)
{
	return (sign(x - split) + 1.0) / 2.0;
}

int get_side()
{
	vec3 f = fract(position);
	if(f.x != 0.0 && f.y != 0.0 && f.z == 0.0)
	{
		return 0;
	}
	if(f.x != 0.0 && f.y == 0.0 && f.z != 0.0)
	{
		return 1;
	}
	if(f.x == 0.0 && f.y != 0.0 && f.z != 0.0)
	{
		return 2;
	}
	return -1;
}

vec2 get_side_coords()
{
	int side = get_side();
	if(side == 0) return position.xy;
	if(side == 1) return position.xz;
	if(side == 2) return position.zy;
	return vec2(0.0);
}
