#include "Cube.hpp"

GLbyte Cube::cube_vertex[] = {
	// back
	 0, 0, 0, // bottom left
	 0, 1, 0, // top left
	 1, 0, 0, // bottom right
	 1, 1, 0, // top right

	// front
	 0, 0, 1, // bottom left
	 0, 1, 1, // top left
	 1, 0, 1, // bottom right
	 1, 1, 1, // top right
};

// note: must be counter-clockwise (relative to front) for back-face culling
uint_fast8_t Cube::cube_elements[] = {
	// back
	0, 1, 2, // bottom left, top left, bottom right
	1, 3, 2, // top left, top right, bottom right

	// front
	6, 5, 4, // bottom right, top left, bottom left
	6, 7, 5, // bottom right, top right, top left

	// top
	1, 5, 3, // back left, front left, back right
	3, 5, 7, // back right, front left, front right

	// bottom
	0, 2, 6, // back left, back right, front right
	0, 6, 4, // back left, front right, front left

	// left
	0, 4, 1, // bottom back, bottom front, top back
	1, 4, 5, // top back, bottom front, top front

	// right
	2, 3, 6, // top back, bottom back, bottom front
	3, 7, 6, // bottom back, top front, bottom front
};
