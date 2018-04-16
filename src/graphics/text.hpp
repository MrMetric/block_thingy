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

#include "graphics/opengl/shader_program.hpp"
#include "graphics/opengl/texture.hpp"
#include "graphics/opengl/vertex_array.hpp"
#include "graphics/opengl/vertex_buffer.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::graphics {

class text
{
public:
	text
	(
		FT_UInt height,
		const fs::path& font_path,
		const fs::path& font_path_bold = {},
		const fs::path& font_path_italic = {},
		const fs::path& font_path_bold_italic = {}
	);
	~text();

	text(text&&) = delete;
	text(const text&) = delete;
	text& operator=(text&&) = delete;
	text& operator=(const text&) = delete;

	enum class font_name
	{
		font0,
		font1,
		font2,
		font3,
		font4,
		font5,
		font6,
		font7,
		font8,
		font9,
		fraktur,
	};
	void set_font(font_name, const fs::path& path);
	void set_font_bold(font_name, const fs::path& path);
	void set_font_italic(font_name, const fs::path& path);
	void set_font_bold_italic(font_name, const fs::path& path);
	void set_font_size(const FT_UInt height);

	void set_projection_matrix(const glm::dmat4& projection_matrix);
	void draw(const std::string&, const glm::dvec2& pos, const glm::dvec4& color = glm::dvec4(1.0));
	void draw(const std::u32string&, const glm::dvec2& pos, const glm::dvec4& color = glm::dvec4(1.0));

	glm::dvec2 get_size(const std::string&);
	glm::dvec2 get_size(std::u32string);

	struct character
	{
		opengl::texture		texture;
		glm::ivec2			size;
		glm::ivec2			bearing;	// offset from baseline to left/top of glyph
		double				x_offset;
		bool				flip;
	};

	static const uint8_t tab_width = 4;

	// this is public so it can be used by static functions in text.cpp
	struct font_data
	{
		character& get_char(char32_t);
		character load_char(char32_t) const;

		fs::path path;
		FT_Face face;
		std::unordered_map<char32_t, character> chars;
	};
private:
	FT_Library ft;
	font_data font_normal;
	std::unique_ptr<font_data> font_bold;
	std::unique_ptr<font_data> font_italic;
	std::unique_ptr<font_data> font_bold_italic;
	FT_UInt height;
	opengl::vertex_buffer vbo;
	opengl::vertex_array vao;
	opengl::shader_program shader;

	struct format_state;
	std::tuple<glm::dvec2, std::vector<double>> loop
	(
		const std::u32string&,
		glm::dvec2,
		format_state&,
		const std::function<void(const glm::dvec2&, character&)>&
	);
};

}
