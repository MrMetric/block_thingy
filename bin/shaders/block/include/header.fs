#version 330

const float M_TAU = 6.283185307179586476925286766559;

in vec3 position;
out vec4 FragColor;
uniform float global_time;

float discretestep(float split, float x)
{
	return (sign(x - split) + 1) / 2;
}

int get_side()
{
	vec3 f = fract(position);

	bool zx = f.z < f.x;
	bool z1x = f.z < 1 - f.x;
	bool zy = f.z < f.y;
	bool z1y = f.z < 1 - f.y;
	if(zx == z1x && zy == z1y)
	{
		return 0;
	}

	bool yx = f.y < f.x;
	bool y1x = f.y < 1 - f.x;
	bool yz = !zy;
	bool y1z = z1y;
	if(yx == y1x && yz == y1z)
	{
		return 1;
	}

	bool xz = !zx;
	bool x1z = z1x;
	bool xy = !yx;
	bool x1y = y1x;
	if(xz == x1z && xy == x1y)
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
	return vec2(0);
}
