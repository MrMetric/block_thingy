uniform float min_light;

vec2 rotate_uv(vec2 uv)
{
	if(rotation == 1) return vec2(uv.y, 1 - uv.x);
	if(rotation == 2) return 1 - uv;
	if(rotation == 3) return vec2(1 - uv.y, uv.x);
	// assume 0
	return uv;
}

void main()
{
	vec2 coords = get_face_coords();
	vec2 uv = fract(coords);
	#ifdef USE_COORDS
	coords = floor(coords) + rotate_uv(uv);
	vec4 c = color(coords);
	#else
	vec2 uv2 = uv;
	if(face == FACE_RIGHT
	|| face == FACE_TOP
	|| face == FACE_BACK)
	{
		uv2.x = 1 - uv2.x;
	}
	uv2 = rotate_uv(uv2);
	vec4 c = color(uv2);
	#endif

	if(face == FACE_TOP || face == FACE_BOTTOM
	|| face == FACE_RIGHT || face == FACE_LEFT)
	{
		uv.xy = uv.yx;
	}
	vec3 light = mix(mix(light1, light2, uv.x), mix(light4, light3, uv.x), uv.y);
	c.rgb *= pow((light * light + min_light) / (1 + min_light), vec3(1 / 2.2));
	FragColor = c;
}
