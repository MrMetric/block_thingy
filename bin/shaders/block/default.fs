#version 330

out vec4 FragColor;
in vec3 position;
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
	if(side == 2) return position.yz;
	return vec2(0.0);
}

vec3 color(vec2 coords)
{
	vec2 c = fract(coords);
	if(c.x < 0.5 && c.y < 0.5
	|| c.x >= 0.5 && c.y >= 0.5)
	{
		return vec3(0.0, 0.0, 0.0);
	}
	return vec3(1.0, 0.0, 1.0);
}

void main()
{
	vec3 c = color(get_side_coords());
	FragColor = vec4(c, 1.0);
}
