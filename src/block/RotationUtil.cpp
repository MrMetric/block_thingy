#include "RotationUtil.hpp"

#include <glm/vec4.hpp>

using Block::Enum::Face;

namespace Block::RotationUtil {

using ivec4 = glm::tvec4<int8_t>;

std::unordered_map<uvec3, std::unordered_map<Face, uint8_t>, Position::hasher_struct<uvec3>> face_rotation_LUT
({
	{{0, 0, 0}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 0}, {Face::back, 0}}},
	{{0, 0, 1}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 3}, {Face::back, 1}}},
	{{0, 0, 2}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 2}, {Face::back, 2}}},
	{{0, 0, 3}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 1}, {Face::back, 3}}},

	{{0, 1, 0}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 0}, {Face::back, 0}}},
	{{0, 1, 1}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 3}, {Face::back, 1}}},
	{{0, 1, 2}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 2}, {Face::back, 2}}},
	{{0, 1, 3}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 1}, {Face::back, 3}}},

	// {0, 2, 0} = {2, 0, 2}
	// {0, 2, 1} = {2, 0, 3}
	// {0, 2, 2} = {2, 0, 0}
	// {0, 2, 3} = {2, 0, 1}

	{{0, 3, 0}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 0}, {Face::back, 0}}},
	{{0, 3, 1}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 3}, {Face::back, 1}}},
	{{0, 3, 2}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 2}, {Face::back, 2}}},
	{{0, 3, 3}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 1}, {Face::back, 3}}},

	{{1, 0, 0}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 0}, {Face::back, 0}}},
	{{1, 0, 1}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 3}, {Face::back, 1}}},
	{{1, 0, 2}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 2}, {Face::back, 2}}},
	{{1, 0, 3}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 1}, {Face::back, 3}}},

	// {1, 1, 0} = {0, 1, 1}
	// {1, 1, 1} = {0, 1, 2}
	// {1, 1, 2} = {0, 1, 3}
	// {1, 1, 3} = {0, 1, 0}

	// {1, 2, 0} = {3, 0, 2}
	// {1, 2, 1} = {3, 0, 3}
	// {1, 2, 2} = {3, 0, 0}
	// {1, 2, 3} = {3, 0, 1}

	// {1, 3, 0} = {0, 3, 3}
	// {1, 3, 1} = {0, 3, 0}
	// {1, 3, 2} = {0, 3, 1}
	// {1, 3, 3} = {0, 3, 2}

	{{2, 0, 0}, {{Face::right, 2}, {Face::left, 2}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 2}, {Face::back, 2}}},
	{{2, 0, 1}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 3}, {Face::bottom, 1}, {Face::front, 1}, {Face::back, 3}}},
	{{2, 0, 2}, {{Face::right, 0}, {Face::left, 0}, {Face::top, 2}, {Face::bottom, 2}, {Face::front, 0}, {Face::back, 0}}},
	{{2, 0, 3}, {{Face::right, 3}, {Face::left, 1}, {Face::top, 1}, {Face::bottom, 3}, {Face::front, 3}, {Face::back, 1}}},

	// {2, 1, 0} = {0, 1, 2}
	// {2, 1, 1} = {0, 1, 3}
	// {2, 1, 2} = {0, 1, 0}
	// {2, 1, 3} = {0, 1, 1}

	// {2, 2, 0} = {0, 0, 2}
	// {2, 2, 1} = {0, 0, 3}
	// {2, 2, 2} = {0, 0, 0}
	// {2, 2, 3} = {0, 0, 1}

	// {2, 3, 0} = {0, 3, 2}
	// {2, 3, 1} = {0, 3, 3}
	// {2, 3, 2} = {0, 3, 0}
	// {2, 3, 3} = {0, 3, 1}

	{{3, 0, 0}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 2}, {Face::back, 2}}},
	{{3, 0, 1}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 1}, {Face::back, 3}}},
	{{3, 0, 2}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 0}, {Face::back, 0}}},
	{{3, 0, 3}, {{Face::right, 1}, {Face::left, 3}, {Face::top, 0}, {Face::bottom, 0}, {Face::front, 3}, {Face::back, 1}}},

	// {3, 1, 0} = {0, 1, 3}
	// {3, 1, 1} = {0, 1, 0}
	// {3, 1, 2} = {0, 1, 1}
	// {3, 1, 3} = {0, 1, 2}

	// {3, 2, 0} = {1, 0, 2}
	// {3, 2, 1} = {1, 0, 3}
	// {3, 2, 2} = {1, 0, 0}
	// {3, 2, 3} = {1, 0, 1}

	// {3, 3, 0} = {0, 3, 1}
	// {3, 3, 1} = {0, 3, 2}
	// {3, 3, 2} = {0, 3, 3}
	// {3, 3, 3} = {0, 3, 0}
});

static int8_t tcos(uint8_t turns)
{
	switch(turns)
	{
		case 0: return  1;
		case 1: return  0;
		case 2: return -1;
		case 3: return  0;
	}
	assert(false);
	return 1;
}

static int8_t tsin(uint8_t turns)
{
	switch(turns)
	{
		case 0: return  0;
		case 1: return  1;
		case 2: return  0;
		case 3: return -1;
	}
	assert(false);
	return 0;
}

static uint8_t tatan2(const int8_t y, const int8_t x)
{
	if(x ==  1 && y ==  0) return 0;
	if(x ==  0 && y ==  1) return 1;
	if(x == -1 && y ==  0) return 2;
	if(x ==  0 && y == -1) return 3;
	if(x ==  0 && y ==  0) return 0;
	throw std::invalid_argument("tatan got x = " + std::to_string(x) + " and y = " + std::to_string(y));
}

static uint8_t mod4(const int8_t turns)
{
	if(turns < 0)
	{
		return 4 - ((-turns) % 4);
	}
	return turns % 4;
}

// derived from glm::rotate
imat4 rotate(uint8_t turns, const ivec3& axis)
{
	if(turns == 0)
	{
		return {};
	}

	turns %= 4;

	const int8_t c = tcos(turns);
	const int8_t s = tsin(turns);

	ivec3 temp(static_cast<int8_t>(1 - c) * axis);

	imat4 m(glm::uninitialize);

	m[0][0] = temp[0] * axis[0] + c;
	m[0][1] = temp[0] * axis[1] + s * axis[2];
	m[0][2] = temp[0] * axis[2] - s * axis[1];

	m[1][0] = temp[1] * axis[0] - s * axis[2];
	m[1][1] = temp[1] * axis[1] + c;
	m[1][2] = temp[1] * axis[2] + s * axis[0];

	m[2][0] = temp[2] * axis[0] + s * axis[1];
	m[2][1] = temp[2] * axis[1] - s * axis[0];
	m[2][2] = temp[2] * axis[2] + c;

	m[3] = {0, 0, 0, 1};

	return m;
}

Enum::Face rotate_face(const Enum::Face face, const uvec3& rot)
{
	if(rot.x == 0 && rot.y == 0 && rot.z == 0)
	{
		return face;
	}

	imat4 r
	 = rotate(rot.x, {1, 0, 0})
	 * rotate(rot.y, {0, 1, 0})
	 * rotate(rot.z, {0, 0, 1});
	return Block::Enum::vec_to_face(glm::ivec3(r * ivec4(Block::Enum::face_to_vec(face), 1)));
}

// derived from glm::extractEulerAngleXYZ
uvec3 mat_to_rot(const imat4& m)
{
	const uint8_t T1 = tatan2(m[2][1], m[2][2]);
	const uint8_t T2 = tatan2(-m[2][0], m[0][0] * m[0][0] + m[1][0] * m[1][0]);
	const int8_t S1 = tsin(T1);
	const int8_t C1 = tcos(T1);
	const uint8_t T3 = tatan2(S1 * m[0][2] - C1 * m[0][1], C1 * m[1][1] - S1 * m[1][2]);
	const uvec3 rot
	{
		mod4(-T1),
		mod4(-T2),
		mod4(-T3),
	};
	return rot;
}

}
