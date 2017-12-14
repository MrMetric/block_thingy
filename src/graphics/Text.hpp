#pragma once

#include <functional>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>
#include <unordered_map>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wdocumentation"
	#pragma clang diagnostic ignored "-Wold-style-cast"
	#pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <ft2build.h>
#include FT_FREETYPE_H
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::graphics {

class Text
{
public:
	Text(const fs::path& font_path, FT_UInt height);
	~Text();

	Text(Text&&) = delete;
	Text(const Text&) = delete;
	Text& operator=(Text&&) = delete;
	Text& operator=(const Text&) = delete;

	void set_font(const fs::path& path, FT_UInt height);
	void set_font(const fs::path& path);
	void set_font_size(const FT_UInt height);

	void set_projection_matrix(const glm::dmat4& projection_matrix);
	void draw(const std::string&, const glm::dvec2& pos, const glm::dvec3& color = glm::dvec3(1.0));
	void draw(const std::u32string&, const glm::dvec2& pos, const glm::dvec3& color = glm::dvec3(1.0));

	glm::dvec2 get_size(const std::string&);
	glm::dvec2 get_size(std::u32string);

	struct Character
	{
		opengl::Texture		texture;
		glm::ivec2			size;
		glm::ivec2			bearing;	// offset from baseline to left/top of glyph
		double				x_offset;
		bool				flip;
	};

	static const uint8_t tab_width = 4;

private:
	FT_Library ft;
	fs::path font_path;
	FT_Face face;
	FT_UInt height;
	std::unordered_map<char32_t, Character> chars;
	opengl::VertexBuffer vbo;
	opengl::VertexArray vao;
	opengl::ShaderProgram shader;

	std::tuple<glm::dvec2, std::vector<double>> loop
	(
		const std::u32string&,
		glm::dvec2,
		std::function<void(const glm::dvec2&, Character&)>
	);
	Character& get_char(char32_t);
	Character load_char(char32_t) const;
};

}
