uniform float min_light;
void main()
{
	vec2 coords = get_face_coords();
	vec2 uv = fract(coords);
	#ifdef USE_COORDS
	vec4 c = color(coords);
	#else
	vec2 uv2 = uv;
	if(face == FACE_FRONT
	|| face == FACE_TOP
	|| face == FACE_LEFT)
	{
		uv2.x = 1 - uv2.x;
	}
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
