#pragma once

#include <ostream>
#include <string>

#define LOG(X) ::block_thingy::logger::log(#X)

namespace block_thingy::logger {

std::ostream& log(const std::string& category);

// https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters
// https://www.ecma-international.org/publications/standards/Ecma-048.htm
// the descriptions are from the ECMA-48 5th edition PDF (available for free at the above link)
enum class format
{
	reset					=  0, // default rendition (implementation-defined)
	bold_intensity			=  1, // bold or increased intensity
	faint_intensity			=  2, // faint, decreased intensity or second colour
	italic					=  3, // italicized
	underline				=  4, // singly underlined
	slow_blink				=  5, // slowly blinking (less then 150 per minute)
	rapid_blink				=  6, // rapidly blinking (150 per minute or more)
	inverse					=  7, // negative image
	conceal					=  8, // concealed characters
	crossed_out				=  9, // crossed-out (characters still legible but marked as to be deleted)

	font_0					= 10, // primary (default) font
	font_1					= 11, // first alternative font
	font_2					= 12, // second alternative font
	font_3					= 13, // third alternative font
	font_4					= 14, // fourth alternative font
	font_5					= 15, // fifth alternative font
	font_6					= 16, // sixth alternative font
	font_7					= 17, // seventh alternative font
	font_8					= 18, // eighth alternative font
	font_9					= 19, // ninth alternative font

	fraktur					= 20, // Fraktur (Gothic)
	double_underline		= 21, // doubly underlined
	normal_intensity		= 22, // normal colour or normal intensity (neither bold nor faint)
	italic_and_fraktur_off	= 23, // not italicized, not fraktur
	underline_off			= 24, // not underlined (neither singly nor doubly)
	blink_off				= 25, // steady (not blinking)
	// 26 = reserved for proportional spacing as specified in CCITT Recommendation T.61
	inverse_off				= 27, // positive image
	conceal_off				= 28, // revealed characters
	crossed_out_off			= 29, // not crossed out

	black					= 30, // black display
	red						= 31, // red display
	green					= 32, // green display
	yellow					= 33, // yellow display
	blue					= 34, // blue display
	magenta					= 35, // magenta display
	cyan					= 36, // cyan display
	white					= 37, // white display
	default_fg				= 39, // default display colour (implementation-defined)

	black_bg				= 40, // black background
	red_bg					= 41, // red background
	green_bg				= 42, // green background
	yellow_bg				= 43, // yellow background
	blue_bg					= 44, // blue background
	magenta_bg				= 45, // magenta background
	cyan_bg					= 46, // cyan background
	white_bg				= 47, // white background
	default_bg				= 49, // default background colour (implementation defined)

	// 50 = reserved for cancelling the effect of the rendering aspect established by parameter value 26
	frame					= 51, // framed
	encircle				= 52, // encircled
	overline				= 53, // overlined
	frame_and_encircle_off	= 54, // not framed, not encircled
	overline_off			= 55, // not overlined
	// 56, 57, 58, 59 = reserved for future standardization

	right_side_line			= 60, // ideogram underline or right side line
	right_side_line_double	= 61, // ideogram double underline or double line on the right side
	left_side_line			= 62, // ideogram overline or left side line
	left_side_line_double	= 63, // ideogram double overline or double left side line
	ideogram_stress_marking	= 64, // ideogram stress marking
	//name?					= 65, // cancels the effect of the rendition aspects established by parameter values 60 to 64

	// aixterm: bright foreground color
	black_bright			= 90,
	red_bright				= 91,
	green_bright			= 92,
	yellow_bright			= 93,
	blue_bright				= 94,
	magenta_bright			= 95,
	cyan_bright				= 96,
	white_bright			= 97,

	// aixterm: bright background color
	black_bright_bg			= 100,
	red_bright_bg			= 101,
	green_bright_bg			= 102,
	yellow_bright_bg		= 103,
	blue_bright_bg			= 104,
	magenta_bright_bg		= 105,
	cyan_bright_bg			= 106,
	white_bright_bg			= 107,
};
std::ostream& operator<<(std::ostream&, format);

}
