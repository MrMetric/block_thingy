#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

template<typename T> using vertex_coord_t = glm::tvec3<T>;
template<typename T> using triangle_t = glm::tvec3<vertex_coord_t<T>>;

// for passing an extra number
template<typename T> using vertex_coord_4_t = glm::tvec4<T>;
template<typename T> using triangle_4_t = glm::tvec3<vertex_coord_4_t<T>>;
