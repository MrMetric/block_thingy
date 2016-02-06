#pragma once

#include <glad/glad.h>
#include <string>

GLuint make_program(const std::string& path);
GLint getUniformLocation(GLuint program, const char* name);
