uniform vec3 light;
const float min_light = 0.06;
void main()
{
	vec2 coords = get_face_coords();
	vec2 uv = fract(coords);
	if(face == FACE_RIGHT
	|| face == FACE_BACK
	|| face == FACE_TOP)
	{
		uv.x = 1 - uv.x;
	}
	vec4 c = color(uv);
	c.rgb *= (light * light + min_light) / (1 + min_light);
	FragColor = c;
}
