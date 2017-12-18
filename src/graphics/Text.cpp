#include "Text.hpp"

#include <algorithm>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>

#include "graphics/OpenGL/Texture.hpp"
#include "util/logger.hpp"
#include "util/unicode.hpp"

using std::string;
using std::u32string;

// based on:
// http://learnopengl.com/#!In-Practice/Text-Rendering
// http://learnopengl.com/#!In-Practice/2D-Game/Render-text

namespace block_thingy::graphics {

text::text(const fs::path& font_path, const FT_UInt height)
:
	font_path(font_path),
	height(height),
	vbo({4, GL_FLOAT}),
	vao(vbo),
	shader("shaders/text")
{
	if(FT_Init_FreeType(&ft))
	{
		throw std::runtime_error("failed to init FreeType");
	}

	if(FT_New_Face(ft, font_path.string().c_str(), 0, &face))
	{
		throw std::runtime_error("failed to load font " + font_path.u8string());
	}

	FT_Set_Pixel_Sizes(face, 0, height);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

text::~text()
{
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void text::set_font(const fs::path& path, const FT_UInt height)
{
	if(path == font_path && height == this->height)
	{
		return;
	}

	if(path != font_path)
	{
		FT_Face new_face;
		if(FT_New_Face(ft, path.string().c_str(), 0, &new_face))
		{
			throw std::runtime_error("failed to load font " + path.u8string());
		}
		FT_Done_Face(face);
		face = new_face;
		font_path = path;
	}
	FT_Set_Pixel_Sizes(face, 0, height);
	this->height = height;
	chars.clear();
}

void text::set_font(const fs::path& path)
{
	if(path == font_path)
	{
		return;
	}

	FT_Face new_face;
	if(FT_New_Face(ft, path.string().c_str(), 0, &new_face))
	{
		throw std::runtime_error("failed to load font " + path.u8string());
	}
	FT_Done_Face(face);
	face = new_face;
	font_path = path;
	FT_Set_Pixel_Sizes(face, 0, height);
	chars.clear();
}

void text::set_font_size(const FT_UInt height)
{
	if(height == this->height)
	{
		return;
	}
	FT_Set_Pixel_Sizes(face, 0, height);
	this->height = height;
	chars.clear();
}

void text::set_projection_matrix(const glm::dmat4& projection_matrix)
{
	shader.uniform("projection", glm::mat4(projection_matrix));
}

void text::draw(const string& s, const glm::dvec2& pos, const glm::dvec3& color)
{
	// TODO: handle exception from invalid input
	draw(util::utf8_to_utf32(s), pos, color);
}

void text::draw(const u32string& s, const glm::dvec2& pos, const glm::dvec3& color)
{
	shader.uniform("color", glm::vec3(color));
	shader.use();
	glActiveTexture(GL_TEXTURE0);

	loop(s, glm::round(pos), [this](const glm::dvec2& pos, character& ch)
	{
		float xpos = static_cast<float>(pos.x + ch.bearing.x);
		float ypos = static_cast<float>(pos.y + (get_char('H').bearing.y - ch.bearing.y));

		const float w = ch.size.x;
		const float h = ch.size.y;
		const float y1 = ch.flip ? 1.0f : 0.0f;
		const float y2 = 1.0f - ch.flip;
		const float vertexes[] =
		{
			xpos,     ypos + h, 0.0f, y2,
			xpos,     ypos,     0.0f, y1,
			xpos + w, ypos,     1.0f, y1,

			xpos,     ypos + h, 0.0f, y2,
			xpos + w, ypos,     1.0f, y1,
			xpos + w, ypos + h, 1.0f, y2,
		};

		glBindTexture(GL_TEXTURE_2D, ch.texture.get_name());

		vbo.data(sizeof(vertexes), vertexes, opengl::vertex_buffer::usage_hint::dynamic_draw);
		vao.draw(GL_TRIANGLES, 0, sizeof(vertexes) / sizeof(vertexes[0]) / 4);
	});
}

glm::dvec2 text::get_size(const string& s_utf8)
{
	// TODO: handle exception from invalid input
	return get_size(util::utf8_to_utf32(s_utf8));
}

glm::dvec2 text::get_size(u32string s)
{
	if(s.empty())
	{
		return {0, 0};
	}
	while(s.back() == '\n' || s.back() == '\t')
	{
		s.pop_back();
	}
	if(s.empty())
	{
		return {0, 0};
	}

	glm::dvec2 size(0, get_char('H').size.y);
	std::vector<double> widths;
	std::tie(size, widths) = loop(s, size, [](const glm::dvec2&, const character&)
	{
	});
	widths.push_back(size.x);

	return
	{
		*std::max_element(widths.cbegin(), widths.cend()),
		size.y,
	};
}

std::tuple<glm::dvec2, std::vector<double>> text::loop
(
	const u32string& s,
	glm::dvec2 pos,
	std::function<void(const glm::dvec2&, character&)> f
)
{
	std::vector<double> widths;
	const double start_x = pos.x;
	uint_fast32_t line_i = 0;
	for(const char32_t c : s)
	{
		if(c == '\n')
		{
			widths.push_back(pos.x);
			pos.x = start_x;
			pos.y += height;
			line_i = 0;
			continue;
		}
		if(c == '\t')
		{
			// TODO?: elastic tabstops
			uint_fast8_t width = tab_width - (line_i % tab_width);
			pos.x += get_char(' ').x_offset * width;
			line_i += width;
			continue;
		}
		++line_i;

		character& ch = get_char(c);

		f(pos, ch);

		pos.x += ch.x_offset;
	}

	return {pos, widths};
}

text::character& text::get_char(const char32_t c)
{
	auto i = chars.find(c);
	if(i == chars.cend())
	{
		chars.emplace(c, load_char(c));
		i = chars.find(c);
	}
	return i->second;
}

text::character text::load_char(const char32_t c) const
{
	if(FT_Load_Char(face, c, FT_LOAD_RENDER))
	{
		// TODO: ensure character is printed legibly
		LOG(ERROR) << "failed to load character '" << c << "' in " << font_path.u8string() << '\n';
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

	character ch
	{
		{
			GL_TEXTURE_2D,
		},
		glm::ivec2(bitmap.width, bitmap.rows),
		glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
		face->glyph->advance.x / 64.0,
		bitmap.pitch < 0,
	};
	ch.texture.image2D
	(
		0,					// level
		GL_RED,				// internal format
		bitmap.width,
		bitmap.rows,
		GL_RED,				// format
		GL_UNSIGNED_BYTE,
		bitmap.buffer
	);
	ch.texture.parameter(opengl::texture::Parameter::wrap_s, GL_CLAMP_TO_EDGE);
	ch.texture.parameter(opengl::texture::Parameter::wrap_t, GL_CLAMP_TO_EDGE);
	ch.texture.parameter(opengl::texture::Parameter::min_filter, GL_LINEAR);
	ch.texture.parameter(opengl::texture::Parameter::mag_filter, GL_LINEAR);
	return ch;
}

}
