#include "copy_stream.hpp"

#include <vector>

using std::string;

namespace Util {

/*
void copy_stream(std::istream& src, std::ostream& dest)
{
}
*/

string read_stream(std::istream& src)
{
	const std::size_t buffer_size = 1024;
	std::vector<char> buf(buffer_size);
	string s;
	std::streamsize read;
	do
	{
		src.read(&buf[0], buffer_size);
		read = src.gcount();
		s.append(&buf[0], static_cast<std::size_t>(read));
	}
	while(read == buffer_size);
	return s;
}

}
