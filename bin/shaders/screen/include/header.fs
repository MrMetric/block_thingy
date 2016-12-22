#version 330 core

const float M_TAU = 6.283185307179586476925286766559;

in vec2 UV;
out vec4 FragColor;
uniform sampler2D tex;
uniform vec2 tex_size;
uniform float time;
