const float min_light = 0.005;
void main()
{
	vec2 coords = get_face_coords();
	vec2 uv = fract(coords);
	if(face == FACE_FRONT
	|| face == FACE_TOP
	|| face == FACE_LEFT)
	{
		uv.x = 1 - uv.x;
	}
	vec4 c = color(uv);
	c.rgb *= pow((light * light + min_light) / (1 + min_light), vec3(1 / 2.2));
	FragColor = c;
}
