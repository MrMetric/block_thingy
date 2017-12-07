#include "ShaderObject.hpp"

#include <sstream>
#include <stdexcept>

#include <glad/glad.h>

#include "Util.hpp"
#include "util/logger.hpp"

using std::string;

namespace Graphics::OpenGL {

string get_log(const GLuint object);
string do_include(const fs::path& file_path);

ShaderObject::ShaderObject()
:
	inited(false),
	name(0)
{
}

ShaderObject::ShaderObject(const fs::path& file_path, GLenum type)
{
	LOG(INFO) << "compiling shader: " << file_path.u8string() << '\n';

	const string source = do_include(file_path);
	const char* source_c = source.c_str();
	const GLint source_len = static_cast<GLint>(source.length());
	name = glCreateShader(type);
	glShaderSource(name, 1, &source_c, &source_len);

	glCompileShader(name);
	GLint compile_ok;
	glGetShaderiv(name, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE)
	{
		string log = Util::gl_object_log(name);
		throw std::runtime_error("error compiling " + file_path.u8string() + ":\n" + log);
	}

	inited = true;
}

ShaderObject::ShaderObject(ShaderObject&& that)
{
	name = that.name;
	inited = that.inited;
	if(inited)
	{
		that.name = 0;
		that.inited = false;
	}
}

ShaderObject::~ShaderObject()
{
	if(inited)
	{
		glDeleteShader(name);
	}
}

GLuint ShaderObject::get_name()
{
	return name;
}

static const string include_str = "#include";
string do_include(const fs::path& file_path)
{
	const string source = Util::read_file(file_path);
	const fs::path folder = file_path.parent_path();

	std::istringstream input(source);
	std::ostringstream output;
	for(string line; std::getline(input, line); )
	{
		if(!Util::string_starts_with(line, include_str))
		{
			output << line << '\n';
			continue;
		}
		string included = line.substr(include_str.length());
		if(included.length() == 0 || (included[0] != ' ' && included[0] != '\t'))
		{
			continue;
		}
		included.erase(0, included.find_first_not_of(" \t"));
		included.erase(included.find_last_not_of(" \t") + 1);
		const fs::path path = folder / included;
		if(!Util::file_is_openable(path))
		{
			LOG(ERROR) << "shader include not found: " << path.u8string() << '\n';
			continue;
		}
		output << do_include(path) << '\n';
	}

	return output.str();
}

}
