#include include/header.fs

#include ../noise/noise2D.glsl

// see http://www.tinysg.de/techGuides/tg1_proceduralMarble.html for more info

const int roughness = 8; // noisiness of veins (#octaves in turbulence)
float turbulence(vec2 P)
{
	float val = 0.0;
	float freq = 1.0;
	for(int i = 0; i < roughness; i++)
	{
		val += abs(snoise(P*freq) / freq);
		freq *= 2.07;
	}
	return val;
}

const float amplitude = 1.0;

vec3 color(vec2 coords)
{
	vec2 n = coords + amplitude * turbulence(coords);
	float c1 = pow(cos(M_TAU / 2.0 * n.x), 1.0/6.0);
	float c2 = pow(cos(M_TAU / 2.0 * n.y), 1.0/4.0);
	vec3 c = vec3((c1 + c2) / 2.0);
	c.r *= 0.975;
	c.b *= 0.975;
	return c;
}

#include include/main.fs
