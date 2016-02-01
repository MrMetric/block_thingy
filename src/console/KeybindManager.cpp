#include "KeybindManager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Console.hpp"

KeybindManager::KeybindManager(const Console* console)
	:
	console(console)
{
}

void KeybindManager::bind_key(int key, const std::string& command)
{
	this->keybinds[key] = command;
}

void KeybindManager::unbind_key(int key)
{
	this->keybinds.erase(key);
}

void KeybindManager::keypress(int key, int action)
{
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		auto i = this->keybinds.find(key);
		if(i != this->keybinds.end())
		{
			std::string command = i->second;
			if(command[0] == '+')
			{
				if(action == GLFW_REPEAT)
				{
					return;
				}
				std::string command2 = command;
				command2[0] = '-';
				this->release_auto[key] = command2;
			}
			this->console->run_command(command);
		}
	}
	else if(action == GLFW_RELEASE)
	{
		auto i = this->release_auto.find(key);
		if(i != this->release_auto.end())
		{
			this->console->run_command(i->second);
		}
	}
	else
	{
		// TODO: log unknown action
	}
}