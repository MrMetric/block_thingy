#version 330

#define M_TAU 6.28318530717958647693

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

// TODO: some sort of #include thing

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
		+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

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

void main()
{
	vec3 c = color(get_side_coords());
	FragColor = vec4(c, 1.0);
}
