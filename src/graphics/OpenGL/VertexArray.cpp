#include "VertexArray.hpp"

VertexArray::VertexArray()
{
	glCreateVertexArrays(1, &name);
	inited = true;
}

VertexArray::VertexArray(VertexArray&& that)
{
	inited = that.inited;
	if(inited)
	{
		name = that.name;
		that.inited = false;
	}
}

VertexArray::~VertexArray()
{
	if(inited)
	{
		glDeleteVertexArrays(1, &name);
		inited = false;
	}
}

GLuint VertexArray::get_name() const
{
	return name;
}

void VertexArray::attrib(GLuint index, bool enabled)
{
	if(enabled)
	{
		glEnableVertexArrayAttrib(name, index);
	}
	else
	{
		glDisableVertexArrayAttrib(name, index);
	}
}
