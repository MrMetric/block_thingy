#version 330

out vec4 FragColor;
in vec3 vPos;
uniform float global_time;

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
	return 1.0 - ((sign(length(fract(vec) - 0.5) - radius) + 1.0) / 2.0);
}

void main()
{
	float c = color(vPos.xy) + color(vPos.xz) + color(vPos.yz);
	vec3 vColor = vec3(c, c, c);
	vColor *= hsv2rgb(vec3(global_time * radius_scaled, 1.0, 1.0));
	vColor *= radius_scaled;
	FragColor = vec4(vColor, 1.0);
}
