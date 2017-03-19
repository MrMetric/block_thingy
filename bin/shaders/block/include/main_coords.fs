uniform vec3 light;
const float min_light = 0.06;
void main()
{
	vec2 coords = get_face_coords();
	vec4 c = color(coords);
	c.rgb *= (light * light + min_light) / (1 + min_light);
	FragColor = c;
}
