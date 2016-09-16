uniform vec3 light;
const float min_light = 0.06;
void main()
{
	vec4 c = color(get_side_coords());
	c.rgb *= (light * light + min_light) / (1.0 + min_light);
	FragColor = c;
}
