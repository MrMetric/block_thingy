#include "RaytraceHit.hpp"

#include "../Coords.hpp"

RaytraceHit::RaytraceHit(Position::BlockInWorld pos, glm::ivec3 face)
	:
	pos(pos),
	face(face)
{
}

Position::BlockInWorld RaytraceHit::adjacent()
{
	return pos + face;
}
