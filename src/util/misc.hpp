#pragma once

#include <algorithm>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

#include <glad/glad.h>

#include "util/filesystem.hpp"

namespace block_thingy::util {

inline bool string_starts_with(const std::string& value, const std::string& start)
{
	if(start.size() > value.size())
	{
		return false;
	}
	return std::equal(start.cbegin(), start.cend(), value.cbegin());
}

// http://stackoverflow.com/a/2072890/1578318
inline bool string_ends_with(const std::string& value, const std::string& ending)
{
	if(ending.size() > value.size())
	{
		return false;
	}
	return std::equal(ending.crbegin(), ending.crend(), value.crbegin());
}

inline std::string strip(std::string s, const std::string& chars)
{
	s.erase(0, s.find_first_not_of(chars));
	s.erase(s.find_last_not_of(chars) + 1);
	return s;
}

inline std::string strip_whitespace(std::string s)
{
	return strip(s, " \t\r\n");
}

inline void replace(std::string& s, const std::string& a, const std::string& b)
{
	auto i = s.find(a);
	while(i != std::string::npos)
	{
		s.replace(i, a.size(), b);
		i = s.find(a, i + b.size());
	}
}

inline void replace_once(std::string& s, const std::string& a, const std::string& b)
{
	if(const auto i = s.find(a); i != std::string::npos)
	{
		s.replace(i, a.size(), b);
	}
}

template<typename T>
inline void delete_element(T& t, typename T::value_type e)
{
	auto i = std::remove(t.begin(), t.end(), e);
	if(i != t.end())
	{
		t.erase(i, t.end());
	}
}

bool file_is_openable(const fs::path&);
std::string read_text(const fs::path&);
std::string read_file(const fs::path&);
std::string read_file(const fs::path&, bool is_text);

std::string gl_object_log(GLuint object);

bool is_integer(const std::string&) noexcept;
std::optional<unsigned int      > stou  (const std::string&) noexcept;
std::optional<unsigned long     > stoul (const std::string&) noexcept;
std::optional<unsigned long long> stoull(const std::string&) noexcept;
std::optional<int               > stoi  (const std::string&) noexcept;
std::optional<long              > stol  (const std::string&) noexcept;
std::optional<long long         > stoll (const std::string&) noexcept;

std::optional<float      > stof (const std::string&) noexcept;
std::optional<double     > stod (const std::string&) noexcept;
std::optional<long double> stold(const std::string&) noexcept;

// TODO: lexical_cast
template<typename T>
std::optional<T> string_to_int(const std::string& s)
{
	static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>, "type must be an integer");

	if constexpr(std::is_unsigned_v<T>)
	{
		if constexpr(sizeof(T) == sizeof(int))
		{
			return util::stou(s);
		}
		if constexpr(sizeof(T) == sizeof(long))
		{
			return util::stoul(s);
		}
		if constexpr(sizeof(T) == sizeof(long long))
		{
			return util::stoull(s);
		}
		const auto ll = util::stoull(s);
		if(ll == std::nullopt
		|| *ll > std::numeric_limits<T>::max())
		{
			return {};
		}
		return *ll;
	}
	else
	{
		if constexpr(sizeof(T) == sizeof(int))
		{
			return util::stoi(s);
		}
		if constexpr(sizeof(T) == sizeof(long))
		{
			return util::stol(s);
		}
		if constexpr(sizeof(T) == sizeof(long long))
		{
			return util::stoll(s);
		}
		const auto ll = util::stoll(s);
		if(ll == std::nullopt
		|| *ll < std::numeric_limits<T>::min()
		|| *ll > std::numeric_limits<T>::max())
		{
			return {};
		}
		return *ll;
	}
}

std::string datetime();

}
