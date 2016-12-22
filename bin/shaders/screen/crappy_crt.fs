#include include/header.fs

void main()
{
	vec2 uv = UV;

	float div = 32.0;
	vec2 offset = (-cos(M_TAU / 2 * (UV + 0.5))) / div;
	uv.x += cos(M_TAU / 2 * uv.x) * offset.y;
	uv.y += cos(M_TAU / 2 * uv.y) * offset.x;
	float m = -1 / (2 / div - 1);
	uv = (uv - 0.5) * m + 0.5;

	if(uv.x < 0 || uv.y < 0
	|| uv.x > 1 || uv.y > 1)
	{
		FragColor = vec4(0, 0, 0, 1);
		return;
	}
	FragColor = texture(tex, uv);
	float y = uv.y * tex_size.y / 4;
	float o = cos(M_TAU * y) + 0.5;
	if(o > 0)
	{
		o /= 3;
	}
	FragColor += 0.2 * o;
}
