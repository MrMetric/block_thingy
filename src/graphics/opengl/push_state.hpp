#pragma once

#include <type_traits>

#include <glad/glad.h>

namespace block_thingy::graphics::opengl {

template<typename T, GLenum pname>
void glGet(T&);

template<typename T, GLenum pname>
void glSet(const T&);

template<typename T, GLenum pname>
void glGet(std::enable_if_t<std::is_same_v<GLboolean, T>, T>& t)
{
	glGetBooleanv(pname, &t);
}

template<typename T, GLenum pname>
void glSet(const std::enable_if_t<std::is_same_v<GLboolean, T>, T>& t)
{
	if(t)
	{
		glEnable(pname);
	}
	else
	{
		glDisable(pname);
	}
}

template<typename T, GLenum pname>
void glGet(std::enable_if_t<std::is_same_v<GLenum, T>, T>& t)
{
	static_assert(sizeof(T) == sizeof(GLint));
	glGetIntegerv(pname, reinterpret_cast<GLint*>(&t));
}

template<>
inline void glSet<GLenum, GL_POLYGON_MODE>(const GLenum& t)
{
	glPolygonMode(GL_FRONT_AND_BACK, t);
}

template<typename T, GLenum pname>
struct push_state
{
	const T new_;
	T old;

	push_state(const T new_)
	:
		new_(new_)
	{
		glGet<T, pname>(old);
		if(new_ == old) return;
		glSet<T, pname>(new_);
	}

	~push_state()
	{
		if(new_ == old) return;
		glSet<T, pname>(old);
	}
};

}
