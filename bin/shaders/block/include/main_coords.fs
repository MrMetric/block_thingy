const float min_light = 0.005;
void main()
{
	vec2 coords = get_face_coords();
	vec4 c = color(coords);
	c.rgb *= pow((light * light + min_light) / (1 + min_light), vec3(1 / 2.2));
	FragColor = c;
}
