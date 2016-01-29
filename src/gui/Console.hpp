#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Console
{
	public:
		Console(GLFWwindow* window);
		virtual ~Console();

		void draw();

	private:
		GLFWwindow* window;
};