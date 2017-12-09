#include "language.hpp"

#include <cassert>
#include <sstream>
#include <unordered_map>

#include "Settings.hpp"
#include "Util.hpp"
#include "util/logger.hpp"

using std::string;

namespace language {

static std::unordered_map<string, std::unordered_map<string, string>> langs
{
	{"en", {}},
};

string get(const string& key)
{
	return get(Settings::get<string>("language"), key);
}

string get(const string& language, const string& key)
{
	auto i = langs.find(language);
	if(i == langs.cend())
	{
		assert(language != "en");
		i = langs.find("en");
	}
	const auto& lang = i->second;
	const auto i2 = lang.find(key);
	if(i2 == lang.cend())
	{
		return key;
	}
	return i2->second;
}

void set(const string& language, const string& key, string value)
{
	auto i = langs.find(language);
	if(i == langs.cend())
	{
		i = langs.emplace(language, std::unordered_map<string, string>()).first;
	}
	i->second.insert_or_assign(key, std::move(value));
}

void load(const string& language, const fs::path& path)
{
	load_text(language, Util::read_text(path));
}

void load_text(const string& language, const string& text)
{
	auto i = langs.find(language);
	if(i == langs.cend())
	{
		i = langs.emplace(language, std::unordered_map<string, string>()).first;
	}
	auto& lang = i->second;
	std::istringstream ss(text);
	for(string line; std::getline(ss, line);)
	{
		const auto pos = line.find('=');
		if(pos == string::npos)
		{
			LOG(WARN) << "invalid line in language text: " << line << '\n';
			continue;
		}
		const string key = Util::strip_whitespace(line.substr(0, pos));
		if(key.empty())
		{
			LOG(WARN) << "invalid line in language text: " << line << '\n';
			continue;
		}
		if(key[0] == '#') // comment
		{
			continue;
		}
		const string value = Util::strip_whitespace(line.substr(pos + 1));
		lang.insert_or_assign(key, value);
	}
}

}
