#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma clang diagnostic pop

#include "graphics/OpenGL/ShaderProgram.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "graphics/OpenGL/VertexArray.hpp"
#include "graphics/OpenGL/VertexBuffer.hpp"

namespace Graphics {

class Text
{
	public:
		Text(const std::string& font_path, FT_UInt height);
		~Text();

		void set_projection_matrix(const glm::dmat4& projection_matrix);
		void draw(const std::string&, const glm::dvec2& pos);
		void draw(const std::u32string&, glm::dvec2 pos);

		glm::dvec2 get_size(const std::string&);
		glm::dvec2 get_size(std::u32string);

		struct Character
		{
			OpenGL::Texture		texture;
			glm::ivec2			size;
			glm::ivec2			bearing;	// offset from baseline to left/top of glyph
			double				x_offset;
			bool				flip;
		};

		static const uint8_t tab_width = 4;

	private:
		FT_Library ft;
		FT_Face face;
		FT_UInt height;
		std::unordered_map<char32_t, Character> chars;
		OpenGL::VertexBuffer vbo;
		OpenGL::VertexArray vao;
		OpenGL::ShaderProgram shader;

		Character& get_char(char32_t);
};

} // namespace Graphics
