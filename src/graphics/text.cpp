#include "text.hpp"

#include <algorithm>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

#include <glad/glad.h>

#include "game.hpp"
#include "Gfx.hpp"
#include "graphics/opengl/texture.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"
#include "util/unicode.hpp"

using std::nullopt;
using std::string;
using std::u32string;

// based on:
// http://learnopengl.com/#!In-Practice/Text-Rendering
// http://learnopengl.com/#!In-Practice/2D-Game/Render-text

namespace block_thingy::graphics {

struct text::format_state
{
	format_state(text&, bool update_shader, const glm::dvec4& color_fg, const glm::dvec4& color_bg);

	void reset();

	void set_color_fg(const glm::dvec4&);
	glm::dvec4 default_color_fg;
	glm::dvec4 color_fg;

	void set_color_bg(const glm::dvec4&);
	glm::dvec4 default_color_bg;
	glm::dvec4 color_bg;

	font_data& get_font() const;
	font_data* font;

	void set_bold(bool);
	void set_faintness(double);
	void set_italic(bool);
	void set_inverse(bool);

	bool bold;
	double faintness;
	bool italic;
	bool underline;
	uint64_t blink_time;
	bool inverse;
	bool conceal;
	bool crossed_out;
	bool double_underline;
	bool overline;

private:
	text& t;
	bool update_shader;
	font_data* _get_font() const;
};

static std::array<glm::dvec4, 256> palette
{{
	// normal colors
	{0  , 0  , 0  , 1}, // black
	{0.6, 0  , 0  , 1}, // red
	{0  , 0.6, 0  , 1}, // green
	{0.6, 0.6, 0  , 1}, // yellow
	{0  , 0  , 0.6, 1}, // blue
	{0.5, 0  , 0.6, 1}, // magenta
	{0  , 0.6, 0.6, 1}, // cyan
	{0.8, 0.8, 0.8, 1}, // white

	// bright colors
	{0.1, 0.1, 0.1, 1}, // black
	{1  , 0  , 0  , 1}, // red
	{0  , 1  , 0  , 1}, // green
	{1  , 1  , 0  , 1}, // yellow
	{0  , 0  , 1  , 1}, // blue
	{1  , 0  , 1  , 1}, // magenta
	{0  , 1  , 1  , 1}, // cyan
	{1  , 1  , 1  , 1}, // white
}};

text::text
(
	const FT_UInt height,
	const fs::path& font_path,
	const fs::path& font_path_bold,
	const fs::path& font_path_italic,
	const fs::path& font_path_bold_italic
)
:
	height(height),
	vbo({4, GL_FLOAT}),
	vao(vbo),
	shader("shaders/text")
{
	static bool palette_inited = false;
	if(!palette_inited)
	{
		palette_inited = true;

		const double vals[]
		{
			0,
			std::pow( 95 / 255.0, 2.2),
			std::pow(135 / 255.0, 2.2),
			std::pow(175 / 255.0, 2.2),
			std::pow(215 / 255.0, 2.2),
			std::pow(255 / 255.0, 2.2),
		};
		for(std::size_t r = 0; r < 6; ++r)
		for(std::size_t g = 0; g < 6; ++g)
		for(std::size_t b = 0; b < 6; ++b)
		{
			const std::size_t i = 16 + 36 * r + 6 * g + b;
			palette[i] = glm::dvec4(vals[r], vals[g], vals[b], 1);
		}

		for(std::size_t i = 0; i < 24; ++i)
		{
			const double val = std::pow((8 + i * 10) / 255.0, 2.2);
			palette[i + 232] = glm::dvec4(val, val, val, 1);
		}
	}

	if(FT_Init_FreeType(&ft))
	{
		throw std::runtime_error("failed to init FreeType");
	}

	auto init_font = [this, height](font_data& font, const fs::path& path) -> void
	{
		if(FT_New_Face(ft, path.string().c_str(), 0, &font.face))
		{
			throw std::runtime_error("failed to load font " + path.u8string());
		}
		FT_Set_Pixel_Sizes(font.face, 0, height);
		font.path = path;
	};

	init_font(font_normal, font_path);

	if(!font_path_bold.empty())
	{
		font_bold = std::make_unique<font_data>();
		init_font(*font_bold, font_path_bold);
	}

	if(!font_path_italic.empty())
	{
		font_italic = std::make_unique<font_data>();
		init_font(*font_italic, font_path_italic);
	}

	if(!font_path_bold_italic.empty())
	{
		font_bold_italic = std::make_unique<font_data>();
		init_font(*font_bold_italic, font_path_bold_italic);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

text::~text()
{
	FT_Done_Face(font_normal.face);
	if(font_bold != nullptr)
	{
		FT_Done_Face(font_bold->face);
	}
	if(font_italic != nullptr)
	{
		FT_Done_Face(font_italic->face);
	}
	if(font_bold_italic != nullptr)
	{
		FT_Done_Face(font_bold_italic->face);
	}
	FT_Done_FreeType(ft);
}

static void set_font_
(
	FT_Library ft,
	const FT_UInt height,
	text::font_data* font,
	const fs::path& path
)
{
	if(font == nullptr || path == font->path)
	{
		return;
	}

	FT_Face new_face;
	if(FT_New_Face(ft, path.string().c_str(), 0, &new_face))
	{
		throw std::runtime_error("failed to load font " + path.u8string());
	}
	FT_Done_Face(font->face);
	font->face = new_face;
	font->path = path;
	FT_Set_Pixel_Sizes(font->face, 0, height);
	font->chars.clear();
}

void text::set_font(const font_name name, const fs::path& path)
{
	set_font_(ft, height, &font_normal, path);
}
void text::set_font_bold(const font_name name, const fs::path& path)
{
	set_font_(ft, height, font_bold.get(), path);
}
void text::set_font_italic(const font_name name, const fs::path& path)
{
	set_font_(ft, height, font_italic.get(), path);
}
void text::set_font_bold_italic(const font_name name, const fs::path& path)
{
	set_font_(ft, height, font_bold_italic.get(), path);
}

void text::set_font_size(const FT_UInt height)
{
	if(height == this->height)
	{
		return;
	}
	this->height = height;

	auto set_height = [height](font_data& font) -> void
	{
		FT_Set_Pixel_Sizes(font.face, 0, height);
		font.chars.clear();
	};

	set_height(font_normal);

	if(font_bold != nullptr)
	{
		set_height(*font_bold);
	}
	if(font_italic != nullptr)
	{
		set_height(*font_italic);
	}
	if(font_bold_italic != nullptr)
	{
		set_height(*font_bold_italic);
	}
}

void text::set_projection_matrix(const glm::dmat4& projection_matrix)
{
	shader.uniform("projection", glm::mat4(projection_matrix));
}

void text::draw(const string& s, const glm::dvec2& pos, const glm::dvec4& color)
{
	// TODO: handle exception from invalid input
	draw(util::utf8_to_utf32(s), pos, color);
}

void text::draw(const u32string& s, const glm::dvec2& pos, const glm::dvec4& color)
{
	format_state state(*this, true, color, {0, 0, 0, 0});

	auto draw_ch = [this](const glm::dvec2& pos, character& ch)
	{
		shader.use();
		glActiveTexture(GL_TEXTURE0);

		const float xpos = static_cast<float>(pos.x);
		const float ypos = static_cast<float>(pos.y);

		// ch.size.x and ch.size.y will surely never be large enough to break this cast (integers lose precision at 2^24)
		const float w = static_cast<float>(ch.size.x);
		const float h = static_cast<float>(ch.size.y);
		const float y1 = ch.flip ? 1.0f : 0.0f;
		const float y2 = 1.0f - ch.flip;
		const float vertexes[]
		{
			xpos + w, ypos,     1.0f, y1,
			xpos,     ypos,     0.0f, y1,
			xpos,     ypos + h, 0.0f, y2,

			xpos,     ypos + h, 0.0f, y2,
			xpos + w, ypos + h, 1.0f, y2,
			xpos + w, ypos,     1.0f, y1,
		};

		glBindTexture(GL_TEXTURE_2D, ch.texture.get_name());
		vbo.data(vertexes, opengl::vertex_buffer::usage_hint::dynamic_draw);
		vao.draw(GL_TRIANGLES, 0, sizeof(vertexes) / sizeof(vertexes[0]) / 4);
	};
	auto draw_character = [this, &state, &draw_ch](const glm::dvec2& pos, character& ch) -> void
	{
		const auto& color_fg = state.inverse ? state.color_bg : state.color_fg;
		const auto& color_bg = state.inverse ? state.color_fg : state.color_bg;

		// assume that H has the max height
		const character& ch_H = state.get_font().get_char('H');
		const double actual_height = ch_H.size.y;
		const double offset = height - actual_height;

		// TODO: per-font bg_pos tweaking
		// Anonymous Pro looks best with pos.y - 3 (when height is 24)
		// Grobe Deutschmeister looks best with no change
		const glm::dvec2 bg_pos(pos.x, pos.y);
		const glm::dvec2 bg_size(ch.x_offset, height);
		Gfx::instance->draw_rectangle(bg_pos, bg_size, color_bg);

		if(state.conceal
		|| (state.blink_time != 0 && game::instance->get_global_ticks() % (state.blink_time * 2) >= state.blink_time))
		{
			return;
		}

		const glm::dvec2 ch_pos
		{
			pos.x + ch.bearing.x,
			pos.y + (ch_H.bearing.y - static_cast<double>(ch.bearing.y)),
		};
		draw_ch(ch_pos, ch);
		if(state.underline && state.double_underline)
		{
			const glm::dvec2 line_pos(pos.x, pos.y + height + 1);
			const glm::dvec2 line_size(ch.x_offset, 3);
			Gfx::instance->draw_rectangle(line_pos, line_size, color_fg);
		}
		else if(state.underline)
		{
			const glm::dvec2 line_pos(pos.x, pos.y + height + 1);
			const glm::dvec2 line_size(ch.x_offset, 1);
			Gfx::instance->draw_rectangle(line_pos, line_size, color_fg);
		}
		else if(state.double_underline)
		{
			const glm::dvec2 line_pos_1(pos.x, pos.y + height + 1);
			const glm::dvec2 line_pos_2(pos.x, pos.y + height + 3);
			const glm::dvec2 line_size(ch.x_offset, 1);
			Gfx::instance->draw_rectangle(line_pos_1, line_size, color_fg);
			Gfx::instance->draw_rectangle(line_pos_2, line_size, color_fg);
		}
		if(state.overline)
		{
			const glm::dvec2 line_pos(pos.x, pos.y + offset - 2);
			const glm::dvec2 line_size(ch.x_offset, 1);
			Gfx::instance->draw_rectangle(line_pos, line_size, color_fg);
		}
		if(state.crossed_out)
		{
			const glm::dvec2 line_pos(pos.x, pos.y + height - 1 - actual_height / 2);
			const glm::dvec2 line_size(ch.x_offset, 1);
			Gfx::instance->draw_rectangle(line_pos, line_size, color_fg);
		}
	};

	loop(s, glm::round(pos), state, draw_character);
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

	format_state state(*this, false, {1, 1, 1, 1}, {0, 0, 0, 0});
	auto draw_character = [](const glm::dvec2&, const character&) -> void
	{
	};

	// assume that H has the max height
	glm::dvec2 size(0, font_normal.get_char('H').size.y); // TODO: I assume using font_normal is not always correct
	std::vector<double> widths;
	std::tie(size, widths) = loop(s, size, state, draw_character);
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
	text::format_state& state,
	const std::function<void(const glm::dvec2&, character&)>& draw_character
)
{
	auto apply_format = [&state](const logger::format fmt) -> void
	{
		if(fmt == logger::format::reset)
		{
			state.reset();
		}

		else if(fmt == logger::format::bold_intensity)  { state.set_bold(true); state.set_faintness(0); }
		else if(fmt == logger::format::faint_intensity) { state.set_bold(false); state.set_faintness(2); }
		else if(fmt == logger::format::italic)          state.set_italic(true);
		else if(fmt == logger::format::underline)       state.underline = true;
		else if(fmt == logger::format::slow_blink)      state.blink_time = 30;
		else if(fmt == logger::format::rapid_blink)     state.blink_time = 10;
		else if(fmt == logger::format::inverse)         state.set_inverse(true);
		else if(fmt == logger::format::conceal)         state.conceal = true;
		else if(fmt == logger::format::crossed_out)     state.crossed_out = true;

		//font0 to font9

		//else if(fmt == logger::format::fraktur)          state.set_fraktur(true);
		else if(fmt == logger::format::double_underline) state.double_underline = true;
		else if(fmt == logger::format::normal_intensity)
		{
			state.set_bold(false);
			state.set_faintness(0);
		}
		else if(fmt == logger::format::italic_and_fraktur_off)
		{
			state.set_italic(false);
			//state.set_fraktur(false);
		}
		else if(fmt == logger::format::underline_off)
		{
			state.underline = false;
			state.double_underline = false;
		}
		else if(fmt == logger::format::blink_off)       state.blink_time = 0;
		else if(fmt == logger::format::inverse_off)     state.set_inverse(false);
		else if(fmt == logger::format::conceal_off)     state.conceal = false;
		else if(fmt == logger::format::crossed_out_off) state.crossed_out = false;

		//frame
		//encircle
		else if(fmt == logger::format::overline)     state.overline = true;
		//frame_and_encircle_off
		else if(fmt == logger::format::overline_off) state.overline = false;

		else if(fmt == logger::format::black)      state.set_color_fg(palette[0]);
		else if(fmt == logger::format::red)        state.set_color_fg(palette[1]);
		else if(fmt == logger::format::green)      state.set_color_fg(palette[2]);
		else if(fmt == logger::format::yellow)     state.set_color_fg(palette[3]);
		else if(fmt == logger::format::blue)       state.set_color_fg(palette[4]);
		else if(fmt == logger::format::magenta)    state.set_color_fg(palette[5]);
		else if(fmt == logger::format::cyan)       state.set_color_fg(palette[6]);
		else if(fmt == logger::format::white)      state.set_color_fg(palette[7]);
		else if(fmt == logger::format::default_fg) state.set_color_fg(state.default_color_fg);

		else if(fmt == logger::format::black_bg)   state.set_color_bg(palette[0]);
		else if(fmt == logger::format::red_bg)     state.set_color_bg(palette[1]);
		else if(fmt == logger::format::green_bg)   state.set_color_bg(palette[2]);
		else if(fmt == logger::format::yellow_bg)  state.set_color_bg(palette[3]);
		else if(fmt == logger::format::blue_bg)    state.set_color_bg(palette[4]);
		else if(fmt == logger::format::magenta_bg) state.set_color_bg(palette[5]);
		else if(fmt == logger::format::cyan_bg)    state.set_color_bg(palette[6]);
		else if(fmt == logger::format::white_bg)   state.set_color_bg(palette[7]);
		else if(fmt == logger::format::default_bg) state.set_color_bg(state.default_color_bg);

		else if(fmt == logger::format::black_bright)      state.set_color_fg(palette[ 8]);
		else if(fmt == logger::format::red_bright)        state.set_color_fg(palette[ 9]);
		else if(fmt == logger::format::green_bright)      state.set_color_fg(palette[10]);
		else if(fmt == logger::format::yellow_bright)     state.set_color_fg(palette[11]);
		else if(fmt == logger::format::blue_bright)       state.set_color_fg(palette[12]);
		else if(fmt == logger::format::magenta_bright)    state.set_color_fg(palette[13]);
		else if(fmt == logger::format::cyan_bright)       state.set_color_fg(palette[14]);
		else if(fmt == logger::format::white_bright)      state.set_color_fg(palette[15]);

		else if(fmt == logger::format::black_bright_bg)   state.set_color_bg(palette[ 8]);
		else if(fmt == logger::format::red_bright_bg)     state.set_color_bg(palette[ 9]);
		else if(fmt == logger::format::green_bright_bg)   state.set_color_bg(palette[10]);
		else if(fmt == logger::format::yellow_bright_bg)  state.set_color_bg(palette[11]);
		else if(fmt == logger::format::blue_bright_bg)    state.set_color_bg(palette[12]);
		else if(fmt == logger::format::magenta_bright_bg) state.set_color_bg(palette[13]);
		else if(fmt == logger::format::cyan_bright_bg)    state.set_color_bg(palette[14]);
		else if(fmt == logger::format::white_bright_bg)   state.set_color_bg(palette[15]);
	};

	std::vector<double> widths;
	const double start_x = pos.x;
	uint_fast32_t line_i = 0;
	for(auto i = s.cbegin(); i != s.cend();)
	{
		font_data& font = state.get_font();
		char32_t c = *i++;
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
			pos.x += font.get_char(' ').x_offset * width;
			line_i += width;
			continue;
		}
		if(c == '\033') // \e
		{
			auto read_ansi = []
			(
				u32string::const_iterator& i,
				const u32string::const_iterator end
			) -> std::optional<std::vector<logger::format>>
			{
				if(i == end || *i++ != '[') return {};
				string digits;
				for(; i != end; ++i)
				{
					const char32_t c32 = *i;
					const string s = util::utf32_to_utf8(c32);
					if(s.size() != 1 || (!std::isdigit(s[0]) && s[0] != ';'))
					{
						break;
					}
					digits += s[0];
				}
				if(i == end || *i++ != 'm') return {};
				digits += ';';
				std::vector<logger::format> numbers;
				std::istringstream ss(digits);
				for(string s; std::getline(ss, s, ';');)
				{
					if(s.empty())
					{
						numbers.emplace_back(logger::format::reset);
					}
					else
					{
						const std::optional<int> i = util::stoi(s);
						if(i != nullopt) // should always be true
						{
							numbers.emplace_back(static_cast<logger::format>(*i));
						}
						else
						{
							LOG(BUG) << "not a number: " << s << '\n';
						}
					}
				}
				return numbers;
			};
			auto i2 = i;
			const auto codes_ = read_ansi(i2, s.cend());
			if(codes_ != nullopt)
			{
				i = i2;
				const auto& codes = *codes_;
				for(auto ci = codes.cbegin(); ci != codes.cend(); ++ci)
				{
					const auto code = *ci;
					if((code == logger::format::foreground_color
					 || code == logger::format::background_color)
					&& ci + 1 != codes.cend())
					{
						const int arg0 = static_cast<int>(*(ci + 1));
						if(arg0 != 2 && arg0 != 5)
						{
							// TODO: test what terminal emulators do
							continue;
						}
						if(arg0 == 2)
						{
							if(ci + 4 >= codes.cend())
							{
								continue;
							}
							ci += 2;
							const int r = static_cast<int>(*ci++);
							const int g = static_cast<int>(*ci++);
							const int b = static_cast<int>(*ci);
							// TODO: I should work on proper color space handling
							const glm::dvec4 color
							{
								std::pow(r / 255.0, 2.2),
								std::pow(g / 255.0, 2.2),
								std::pow(b / 255.0, 2.2),
								1,
							};
							if(code == logger::format::foreground_color)
							{
								state.set_color_fg(color);
							}
							else
							{
								assert(code == logger::format::background_color);
								state.set_color_bg(color);
							}
						}
						else
						{
							assert(arg0 == 5);
							if(ci + 2 == codes.cend())
							{
								continue;
							}
							ci += 2;
							const int p = static_cast<int>(*ci);
							if(p < 0 || static_cast<std::size_t>(p) >= palette.size())
							{
								continue;
							}
							const std::size_t i = static_cast<std::size_t>(p);
							if(code == logger::format::foreground_color)
							{
								state.set_color_fg(palette[i]);
							}
							else
							{
								state.set_color_bg(palette[i]);
							}
						}
						continue;
					}
					apply_format(code);
				}
				continue;
			}
		}
		++line_i;

		character& ch = font.get_char(c);

		draw_character(pos, ch);

		pos.x += ch.x_offset;
	}

	return {pos, widths};
}

text::character& text::font_data::get_char(const char32_t c)
{
	auto i = chars.find(c);
	if(i == chars.cend())
	{
		chars.emplace(c, load_char(c));
		i = chars.find(c);
	}
	return i->second;
}

text::character text::font_data::load_char(const char32_t c) const
{
	if(FT_Load_Char(face, c, FT_LOAD_RENDER))
	{
		// TODO: ensure character is printed legibly
		LOG(ERROR) << "failed to load character '" << c << "' in " << path.u8string() << '\n';
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

text::format_state::format_state
(
	text& t,
	bool update_shader,
	const glm::dvec4& color_fg,
	const glm::dvec4& color_bg
)
:
	default_color_fg(color_fg),
	color_fg(color_fg),
	default_color_bg(color_bg),
	color_bg(color_bg),
	font(&t.font_normal),
	bold(false),
	faintness(0),
	italic(false),
	underline(false),
	blink_time(0),
	inverse(false),
	conceal(false),
	crossed_out(false),
	double_underline(false),
	overline(false),
	t(t),
	update_shader(update_shader)
{
	if(update_shader)
	{
		t.shader.uniform("color", glm::vec4(color_fg));
	}
}

void text::format_state::reset()
{
	set_color_fg(default_color_fg);
	set_color_bg(default_color_bg);
	set_bold(false);
	set_faintness(0);
	set_italic(false);
	underline = false;
	blink_time = 0;
	set_inverse(false);
	conceal = false;
	crossed_out = false;
	double_underline = false;
	overline = false;
}

void text::format_state::set_color_fg(const glm::dvec4& c)
{
	if(color_fg == c)
	{
		return;
	}
	color_fg = c;
	if(update_shader && !inverse)
	{
		t.shader.uniform("color", glm::vec4(c));
	}
}

void text::format_state::set_color_bg(const glm::dvec4& c)
{
	if(color_bg == c)
	{
		return;
	}
	color_bg = c;
	if(update_shader && inverse)
	{
		t.shader.uniform("color", glm::vec4(c));
	}
}

text::font_data& text::format_state::get_font() const
{
	return *font;
}

void text::format_state::set_bold(const bool b)
{
	if(bold == b)
	{
		return;
	}
	bold = b;
	font = _get_font();
}

void text::format_state::set_faintness(const double f)
{
	if(f == faintness)
	{
		return;
	}
	faintness = f;
	if(update_shader)
	{
		t.shader.uniform("faintness", static_cast<float>(f));
	}
}

void text::format_state::set_italic(const bool b)
{
	if(italic == b)
	{
		return;
	}
	italic = b;
	font = _get_font();
}

void text::format_state::set_inverse(const bool b)
{
	if(inverse == b)
	{
		return;
	}
	inverse = b;
	if(update_shader)
	{
		t.shader.uniform("color", glm::vec4(b ? color_bg : color_fg));
	}
}

text::font_data* text::format_state::_get_font() const
{
	if(bold)
	{
		if(italic)
		{
			if(t.font_bold_italic != nullptr)
			{
				return t.font_bold_italic.get();
			}
			if(t.font_bold != nullptr)
			{
				return t.font_bold.get();
			}
			if(t.font_italic != nullptr)
			{
				return t.font_italic.get();
			}
			return &t.font_normal;
		}
		if(t.font_bold != nullptr)
		{
			return t.font_bold.get();
		}
		return &t.font_normal;
	}
	if(italic && t.font_italic != nullptr)
	{
		return t.font_italic.get();
	}
	return &t.font_normal;
}

}
