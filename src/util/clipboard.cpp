#include "clipboard.hpp"

#include <GLFW/glfw3.h>

#include "Gfx.hpp"

using std::string;

namespace Util::Clipboard {

string get_text()
{
	const char* text = glfwGetClipboardString(Gfx::instance->window);
	if(text == nullptr)
	{
		return "";
	}
	return text;
}

void set_text(const string& s)
{
	glfwSetClipboardString(Gfx::instance->window, s.c_str());
}

}
