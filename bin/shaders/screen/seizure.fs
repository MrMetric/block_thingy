#include include/header.fs

const float interval = 0.1;

void main()
{
	vec2 offset_m = sin(global_time / interval) / tex_size;
	vec2 offset_r = vec2(-4,  0) * offset_m;
	vec2 offset_g = vec2( 0, -4) * offset_m;
	vec2 offset_b = vec2( 2,  2) * offset_m;

	float m = mod(global_time, interval * 2) / (interval * 2) * 0.1 + 1;
	vec2 uv = ((UV - 0.5) / m) + 0.5;
	FragColor = vec4
	(
		texture(tex, uv + offset_r).r,
		texture(tex, uv + offset_g).g,
		texture(tex, uv + offset_b).b,
		texture(tex, uv).a
	);
	if(mod(global_time, interval) - interval / 2 < 0)
	{
		FragColor = vec4(1 - FragColor.rgb, 1);
	}
}
