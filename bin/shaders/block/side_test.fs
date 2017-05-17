#include include/header.fs

vec4 color(vec2 uv)
{
	float c = pow((cos(M_TAU * uv.x) + 1) / 2, 0.25);
	if(face == FACE_TOP || face == FACE_BOTTOM)
	{
		c *= pow((cos(M_TAU * uv.y) + 1) / 2, 0.25);
	}

	if(face == FACE_RIGHT)
	{
		return vec4(0, 0, c, 1);
	}
	if(face == FACE_LEFT)
	{
		return vec4(c, c, 0, 1);
	}

	if(face == FACE_TOP)
	{
		return vec4(0, c, 0, 1);
	}
	if(face == FACE_BOTTOM)
	{
		return vec4(c, 0, c, 1);
	}

	if(face == FACE_FRONT)
	{
		return vec4(c, 0, 0, 1);
	}
	if(face == FACE_BACK)
	{
		return vec4(0, c, c, 1);
	}

	// since one of the above checks will always match, it is safe to not return here
	// also, my NVIDIA GPU has a bug that makes it skip the last check, so the left face was always black
}

#include include/main.fs
