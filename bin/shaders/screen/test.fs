#include include/header.fs

void main()
{
	vec2 uv = UV;

	uv.x += cos(20 * UV.y + global_time) / 32;
	uv.y += cos(20 * UV.x + global_time) / 32;

	if(uv.x < 0 || uv.y < 0
	|| uv.x > 1 || uv.y > 1)
	{
		FragColor = vec4(0, 0, 0, 1);
		return;
	}
	FragColor = exp(M_TAU / 4 * texture(tex, uv)) - 1;
}
