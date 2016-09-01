#include "Text.hpp"

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>

#include "graphics/OpenGL/Texture.hpp"

using std::cerr;
using std::string;
using std::u32string;

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;

// based on:
// http://learnopengl.com/#!In-Practice/Text-Rendering
// http://www.learnopengl.com/#!In-Practice/2D-Game/Render-text

namespace Graphics {

Text::Character load_char(const FT_Face& face, const FT_ULong c);

Text::Text(const string& font_path, const FT_UInt height)
	:
	height(height),
	vbo({4, GL_FLOAT}),
	vao(vbo),
	shader("shaders/text")
{
	if(FT_Init_FreeType(&ft))
	{
		throw std::runtime_error("failed to init FreeType");
	}

	if(FT_New_Face(ft, font_path.c_str(), 0, &face))
	{
		throw std::runtime_error("failed to load font " + font_path);
	}

	FT_Set_Pixel_Sizes(face, 0, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	chars.emplace('H', load_char(face, 'H'));

	shader.uniform("color", glm::vec3(1.0));
}

Text::~Text()
{
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Text::set_projection_matrix(const glm::dmat4& projection_matrix)
{
	shader.uniform("projection", glm::mat4(projection_matrix));
}

void Text::draw(const string& s_utf8, const glm::dvec2& pos)
{
	// TODO: handle exception from invalid input
	u32string s = convert.from_bytes(s_utf8);
	draw(s, pos);
}

void Text::draw(const u32string& s, glm::dvec2 pos)
{
	const double start_x = pos.x;

	glUseProgram(shader.get_name());

	glActiveTexture(GL_TEXTURE0);

	uint_fast32_t line_i = 0;
	for(const char32_t c : s)
	{
		if(c == '\n')
		{
			pos.x = start_x;
			pos.y += height;
			line_i = 0;
			continue;
		}
		if(c == '\t')
		{
			// TODO?: elastic tabstops
			uint_fast8_t width = tab_width - (line_i % tab_width);
			pos.x += chars[' '].x_offset * width;
			line_i += width;
			continue;
		}
		++line_i;

		Character& ch = get_char(c);

		float xpos = static_cast<float>(pos.x + ch.bearing.x);
		float ypos = static_cast<float>(pos.y + (chars['H'].bearing.y - ch.bearing.y));

		const float w = ch.size.x;
		const float h = ch.size.y;
		const float y1 = ch.flip ? 1.0f : 0.0f;
		const float y2 = 1.0f - ch.flip;
		const float vertices[] =
		{
			xpos,     ypos + h, 0.0f, y2,
			xpos,     ypos,     0.0f, y1,
			xpos + w, ypos,     1.0f, y1,

			xpos,     ypos + h, 0.0f, y2,
			xpos + w, ypos,     1.0f, y1,
			xpos + w, ypos + h, 1.0f, y2,
		};

		glBindTexture(GL_TEXTURE_2D, ch.texture.get_name());

		vbo.data(sizeof(vertices), vertices, OpenGL::VertexBuffer::UsageHint::dynamic_draw);
		vao.draw(GL_TRIANGLES, 0, 6);

		pos.x += ch.x_offset;
	}
}

// TODO: deduplicate with draw
glm::dvec2 Text::get_size(const string& s_utf8)
{
	// TODO: handle exception from invalid input
	u32string s = convert.from_bytes(s_utf8);
	return get_size(s);
}

glm::dvec2 Text::get_size(u32string s)
{
	while(s.back() == '\n' || s.back() == '\t')
	{
		s.pop_back();
	}
	if(s.size() == 0)
	{
		return {0, 0};
	}

	uint_fast32_t line_i = 0;
	std::vector<double> widths;
	glm::dvec2 size(0, chars['H'].size.y);
	for(decltype(s)::size_type i = 0; i < s.length(); ++i)
	{
		const char32_t c = s[i];
		if(c == '\n')
		{
			widths.push_back(size.x);
			size.x = 0;
			size.y += height;
			line_i = 0;
			continue;
		}
		if(c == '\t')
		{
			// TODO?: elastic tabstops
			uint_fast8_t width = tab_width - (line_i % tab_width);
			size.x += chars[' '].x_offset * width;
			line_i += width;
			continue;
		}
		++line_i;

		const Character& ch = get_char(c);

		if(i != s.length() - 1)
		{
			size.x += ch.x_offset;
		}
		else
		{
			size.x += ch.size.x;
		}
	}
	widths.push_back(size.x);

	return {*std::max_element(widths.cbegin(), widths.cend()), size.y};
}

Text::Character& Text::get_char(const char32_t c)
{
	auto ci = chars.find(c);
	if(ci == chars.cend())
	{
		chars.emplace(c, load_char(face, c));
		ci = chars.find(c);
	}
	return ci->second;
}

Text::Character load_char(const FT_Face& face, const FT_ULong c)
{
	if(FT_Load_Char(face, c, FT_LOAD_RENDER))
	{
		cerr << "failed to load character: " << c << "\n";
		return
		{
			{},
			{0, 0},
			{0, 0},
			0,
			false,
		};
	}

	const auto& bitmap = face->glyph->bitmap;

	return
	{
		{
			GL_TEXTURE_2D,
			bitmap.buffer,
			bitmap.width,
			bitmap.rows,
			GL_UNSIGNED_BYTE,
		},
		glm::ivec2(bitmap.width, bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		face->glyph->advance.x / 64.0,
		bitmap.pitch < 0,
	};
}

} // namespace Graphics
