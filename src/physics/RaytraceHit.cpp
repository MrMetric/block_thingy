#include "RaytraceHit.hpp"

Position::BlockInWorld RaytraceHit::adjacent()
{
	return this->pos + this->face;
}