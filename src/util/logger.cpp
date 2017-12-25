#include "logger.hpp"

#include <iostream>

#ifdef HAVE_POSIX
	#include <unistd.h>
#endif

#include "util/misc.hpp"

using std::cerr;
using std::cout;
using std::string;

namespace block_thingy::logger {

class nullbuf : public std::streambuf
{
public:
	int_type overflow(int_type c) override;
};
std::streambuf::int_type nullbuf::overflow(std::streambuf::int_type c)
{
	return c;
}

std::ostream& log(const string& category)
{
	#ifndef BT_DEBUG_BUILD
	if(category == "DEBUG")
	{
		static nullbuf null_buffer;
		static std::ostream null_stream(&null_buffer);
		return null_stream;
	}
	#endif

	std::ostream& o = (category == "ERROR") ? cerr : cout;

	o << format::reset << util::datetime() << ' ';
	if(category == "INFO")
	{
		// no formatting
	}
#ifdef BT_DEBUG_BUILD
	else if(category == "DEBUG")
	{
		o << format::blue;
	}
#endif
	else if(category == "WARN")
	{
		o << format::yellow;
	}
	else if(category == "ERROR")
	{
		o << format::bold_intensity << format::red;
	}
	else if(category == "BUG")
	{
		o << format::bold_intensity << format::red_bg;
	}
	o << category << format::reset;
	if(category.size() < 5)
	{
		o << string(5 - category.size(), ' ');
	}
	o << ' ';

	return o;
}

std::ostream& operator<<(std::ostream& o, [[maybe_unused]] const format fmt)
{
#ifdef _WIN32
	// not supported on Windows, except for recent versions of Windows 10
	return o;
#else
	#ifdef HAVE_POSIX
	if(&o == &cout && !isatty(STDOUT_FILENO)
	|| &o == &cerr && !isatty(STDERR_FILENO))
	{
		return o;
	}
	#endif

	return o << "\033[" << static_cast<int>(fmt) << 'm';
#endif
}

}
