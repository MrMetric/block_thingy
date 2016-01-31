#include <stdexcept>
#include <string>
#include <vector>

class ArgumentParser
{
	public:
		ArgumentParser();

		std::vector<std::string> parse_args(const std::string& argline);

	private:
		std::string::const_iterator iterator;
		std::string::const_iterator end;

		std::string read_string(const char endchar);
};

class truncated_argument : public std::runtime_error
{
	public:
		truncated_argument(const char* message) : std::runtime_error(message)
		{
		}
};