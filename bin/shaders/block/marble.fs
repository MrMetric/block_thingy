#include include/header.fs

#include ../noise/noise2D.glsl

// see http://www.tinysg.de/techGuides/tg1_proceduralMarble.html for more info

float sum_octaves(vec2 P)
{
	const int octave_count = 8;
	float val = 0;
	float freq = 1;
	for(int i = 0; i < octave_count; i++)
	{
		val += abs(snoise(P * freq) / freq);
		freq *= 2.07;
	}
	return val;
}

const float amplitude = 1;

vec4 color(vec2 coords)
{
	vec2 n = coords + amplitude * sum_octaves(coords);
	float x = pow(abs(cos(M_TAU / 2 * n.x)), 1.0 / 6.0);
	float y = pow(abs(cos(M_TAU / 2 * n.y)), 1.0 / 4.0);
	vec3 c = vec3((x + y) / 2);
	c.r *= 0.975;
	c.b *= 0.975;
	return vec4(c, 1);
}

#include include/main_coords.fs
