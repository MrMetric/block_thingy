#include "shader_object.hpp"

#include <sstream>
#include <stdexcept>
#include <vector>

#include <glad/glad.h>

#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::opengl {

string get_log(const GLuint object);
string do_include(const fs::path& file_path, std::size_t file_number, std::vector<fs::path>& file_paths);

shader_object::shader_object()
:
	inited(false),
	name(0)
{
}

shader_object::shader_object(const fs::path& file_path, GLenum type)
{
	LOG(INFO) << "compiling shader: " << file_path.u8string() << '\n';

	std::vector<fs::path> file_paths;
	file_paths.emplace_back(file_path);
	const string source = do_include(file_path, 0, file_paths);
	const char* source_c = source.c_str();
	const GLint source_len = static_cast<GLint>(source.length());
	name = glCreateShader(type);
	glShaderSource(name, 1, &source_c, &source_len);

	glCompileShader(name);
	const string log = util::gl_object_log(name);
	GLint compile_ok;
	glGetShaderiv(name, GL_COMPILE_STATUS, &compile_ok);
	if(!compile_ok)
	{
		std::ostringstream ss;
		ss << "error compiling " << file_path.u8string() << ":\n";
		ss << log << '\n';
		ss << "file paths:\n";
		for(std::size_t i = 0; i < file_paths.size(); ++i)
		{
			ss << i << '\t' << file_paths[i].u8string() << '\n';
		}
		string s = ss.str();
		s.pop_back(); // delete trailing \n
		throw std::runtime_error(s);
	}
	if(!log.empty())
	{
		LOG(DEBUG) << "info log for " << file_path.u8string() << ":\n" << log << '\n';
	}

	inited = true;
}

shader_object::shader_object(shader_object&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

shader_object::~shader_object()
{
	if(inited)
	{
		glDeleteShader(name);
	}
}

GLuint shader_object::get_name()
{
	return name;
}

static const string include_str = "#include";
string do_include(const fs::path& file_path, std::size_t file_number, std::vector<fs::path>& file_paths)
{
	const string source = util::read_text(file_path);
	const fs::path directory = file_path.parent_path();

	std::istringstream input(source);
	std::ostringstream output;
	std::size_t line_number = 1;
	for(string line; std::getline(input, line); ++line_number)
	{
		if(file_number != 0 && line_number == 1)
		{
			if(util::string_starts_with(line, "#version"))
			{
				output << line << '\n';
				output << "#line 2 " << file_number << " // " << file_path.u8string() << '\n';
				continue;
			}
			output << "#line 1 " << file_number << " // " << file_path.u8string() << '\n';
		}
		if(!util::string_starts_with(line, include_str))
		{
			output << line << '\n';
			continue;
		}
		string included = line.substr(include_str.length());
		if(included.length() == 0 || (included[0] != ' ' && included[0] != '\t'))
		{
			continue;
		}
		included = util::strip_whitespace(included);
		const fs::path path = directory / included;
		if(!fs::exists(path))
		{
			LOG(ERROR) << "shader include not found: " << path.u8string() << '\n';
			continue;
		}
		const auto inc_i = std::find(file_paths.cbegin(), file_paths.cend(), path);
		const std::size_t inc_file_number = static_cast<std::size_t>(inc_i - file_paths.cbegin());
		if(inc_i == file_paths.cend())
		{
			file_paths.emplace_back(path);
		}
		output << do_include(path, inc_file_number, file_paths) << '\n';
		output << "#line " << line_number + 1 << ' ' << file_number << " // " << file_path.u8string() << '\n';
	}

	return output.str();
}

}
