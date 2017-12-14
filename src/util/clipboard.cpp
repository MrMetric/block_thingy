#include "clipboard.hpp"

#include <GLFW/glfw3.h>

#include "Gfx.hpp"

#include "util/unicode.hpp"

using std::string;

namespace block_thingy::util::clipboard {

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

void set_text(const std::u32string& s)
{
	set_text(utf32_to_utf8(s));
}

}
