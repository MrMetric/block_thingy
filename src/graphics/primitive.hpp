#pragma once

#include <glm/vec3.hpp>

template<typename T> using vertex_coord_t = glm::tvec3<T>;
template<typename T> using triangle_t = glm::tvec3<vertex_coord_t<T>>;
