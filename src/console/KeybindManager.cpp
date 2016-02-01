#include "KeybindManager.hpp"

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

void KeybindManager::keypress(int key) const
{
	auto i = this->keybinds.find(key);
	if(i != this->keybinds.end())
	{
		this->console->run_command(i->second);
	}
}