#include "ShaderObject.hpp"

#include <sstream>
#include <stdexcept>

#include <glad/glad.h>

#include "Util.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::graphics::opengl {

string get_log(const GLuint object);
string do_include(const fs::path& file_path);

shader_object::shader_object()
:
	inited(false),
	name(0)
{
}

shader_object::shader_object(const fs::path& file_path, GLenum type)
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
		string log = util::gl_object_log(name);
		throw std::runtime_error("error compiling " + file_path.u8string() + ":\n" + log);
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
string do_include(const fs::path& file_path)
{
	const string source = util::read_text(file_path);
	const fs::path folder = file_path.parent_path();

	std::istringstream input(source);
	std::ostringstream output;
	for(string line; std::getline(input, line);)
	{
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
		const fs::path path = folder / included;
		if(!fs::exists(path))
		{
			LOG(ERROR) << "shader include not found: " << path.u8string() << '\n';
			continue;
		}
		output << do_include(path) << '\n';
	}

	return output.str();
}

}
