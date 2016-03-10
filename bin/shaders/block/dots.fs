#version 330

out vec4 FragColor;
in vec3 vPos;
uniform float global_time;

float discretestep(float split, float x)
{
	return (sign(x - split) + 1.0) / 2.0;
}

int get_side()
{
	vec3 f = fract(vPos);
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
	if(side == 0) return vPos.xy;
	if(side == 1) return vPos.xz;
	if(side == 2) return vPos.yz;
	return vec2(0.0);
}

float radius = 0.3 * (sin(4.0 * global_time + length(vPos))/4.0 + 1.0);
const float radius_max = 0.3 * (1 / 4.0 + 1.0);
float radius_scaled = radius / radius_max;

// http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float color(vec2 vec)
{
	return 1.0 - discretestep(radius, length(fract(vec) - 0.5));
}

void main()
{
	vec3 c = vec3(color(get_side_coords()));
	c *= hsv2rgb(vec3(global_time + radius_scaled, 1.0, 1.0));
	c *= radius_scaled;
	FragColor = vec4(c, 1.0);
}
