#include "logger.hpp"

#include "Util.hpp"

using std::cerr;
using std::cout;
using std::string;

namespace block_thingy {

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
	#ifndef DEBUG_BUILD
	if(category == "DEBUG")
	{
		static nullbuf null_buffer;
		static std::ostream null_stream(&null_buffer);
		return null_stream;
	}
	#endif

	std::ostream& o = (category == "ERROR") ? cerr : cout;

	o << Util::datetime() << ' ' << category;
	if(category.size() < 5)
	{
		o << string(5 - category.size(), ' ');
	}
	o << ' ';

	return o;
}

}
