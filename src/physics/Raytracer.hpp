#pragma once

#include <cstdint>
#include <memory>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "RaytraceHit.hpp"

class World;

class Raytracer
{
	public:
		static void ScreenPosToWorldRay(
			uint_fast32_t mouseX, uint_fast32_t mouseY,
			uint_fast32_t screenWidth, uint_fast32_t screenHeight,
			glm::dmat4 ViewMatrix,
			glm::dmat4 ProjectionMatrix,
			glm::dvec3& out_origin,
			glm::dvec3& out_direction
		);
		static std::unique_ptr<RaytraceHit> raycast(const World& world, glm::dvec3 origin, glm::dvec3 direction, double radius);
};
