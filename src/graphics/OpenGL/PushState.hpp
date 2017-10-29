#pragma once

#include <type_traits>

#include <glad/glad.h>

namespace Graphics::OpenGL {

template<typename T>
void glGet(const GLenum pname, std::enable_if_t<std::is_same<GLboolean, T>::value, T>& t)
{
	glGetBooleanv(pname, &t);
}

template<typename T>
void glSet(const GLenum pname, const std::enable_if_t<std::is_same<GLboolean, T>::value, T>& t)
{
	if(t) glEnable(pname);
	else glDisable(pname);
}

template<typename T>
struct PushState
{
	const GLenum pname;
	const T new_;
	T old;

	PushState(const GLenum pname, const T new_)
	:
		pname(pname),
		new_(new_)
	{
		glGet<T>(pname, old);
		if(new_ == old) return;
		glSet<T>(pname, new_);
	}

	~PushState()
	{
		if(new_ == old) return;
		glSet<T>(pname, old);
	}
};

}
