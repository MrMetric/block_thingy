#pragma once

#include <stdint.h>
#include <unordered_map>

#include <glad/glad.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#pragma clang diagnostic ignored "-Wold-style-cast"
#include <ft2build.h>
#include FT_TYPES_H
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

		void set_projection_matrix(const glm::dmat4& projection_matrix);
		void draw(const std::string&, glm::dvec2 pos);

		glm::dvec2 get_size(std::string);

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
		FT_UInt height;
		std::unordered_map<char, Character> chars;
		OpenGL::VertexBuffer vbo;
		OpenGL::VertexArray vao;
		OpenGL::ShaderProgram shader;
};

} // namespace Graphics
