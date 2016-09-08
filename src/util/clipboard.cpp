#include "clipboard.hpp"

#include <GLFW/glfw3.h>

#include "Game.hpp"
#include "Gfx.hpp"

using std::string;

namespace Util::Clipboard {

string get_text()
{
	const char* text = glfwGetClipboardString(Game::instance->gfx.window);
	if(text == nullptr)
	{
		return "";
	}
	return text;
}

void set_text(const string& s)
{
	glfwSetClipboardString(Game::instance->gfx.window, s.c_str());
}

}
