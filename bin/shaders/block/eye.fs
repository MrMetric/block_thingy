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
	if(side == 2) return position.zy;
	return vec2(0.0);
}

vec3 color(vec2 vec)
{
	float offset = sin(global_time) + sin(2.0 * global_time) + sin(3.0 * global_time);
	float len = length(fract(vec) - 0.5);
	float m = 999999.0;
	float r = (cos(      offset + m * len) + 1.0) / 2.0;
	float g = (cos(0.1 + offset + m * len) + 1.0) / 2.0;
	float b = (cos(0.2 + offset + m * len) + 1.0) / 2.0;
	return vec3(r, g, b);
}

void main()
{
	vec3 c = color(get_side_coords());
	FragColor = vec4(c, 1.0);
}
