void main()
{
	vec3 c = color(get_side_coords());
	FragColor = vec4(c, 1.0);
}
