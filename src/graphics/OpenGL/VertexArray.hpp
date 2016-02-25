#pragma once

#include <glad/glad.h>

class VertexArray
{
	public:
		VertexArray();
		VertexArray(const VertexArray&) = delete;
		VertexArray(VertexArray&&);
		~VertexArray();

		VertexArray& operator=(const VertexArray&) = delete;

		GLuint get_name() const;
		void attrib(GLuint index, bool enabled);

	private:
		bool inited;
		GLuint name;
};
