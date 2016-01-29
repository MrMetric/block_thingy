#pragma once

void print_log(GLuint object);
GLuint compile_shader(const std::string& filename, GLenum type);
GLuint make_program(const std::string& path);
GLint getUniformLocation(GLuint program, const char* name);