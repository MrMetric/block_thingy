#include "image.hpp"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-macros"
#endif
#define PNG_SKIP_SETJMP_CHECK // for libpng < 1.5
#if defined(__clang__)
	#pragma clang diagnostic pop
#endif
#include <png.h>

#include "util/logger.hpp"

using std::string;
using std::unique_ptr;

namespace block_thingy::graphics {

static std::tuple<std::vector<uint8_t>, uint32_t, uint32_t> read_png(const fs::path&);
static void write_png
(
	const fs::path&,
	const std::vector<uint8_t>& data,
	uint32_t width,
	uint32_t height
);

static void write_raw
(
	const fs::path&,
	const std::vector<uint8_t>& data
);

struct image::impl
{
	impl()
	:
		width(0),
		height(0),
		has_transparency(false)
	{
	}

	explicit impl(const fs::path& path)
	{
		if(path.extension() == ".png")
		{
			std::tie(data, width, height) = read_png(path);
		}
		else
		{
			throw std::invalid_argument("unknown image format: " + path.extension().u8string());
		}
		assert(data.size() == 4 * width * height);
		check_transparency();
	}

	impl(const uint32_t width, const uint32_t height, std::vector<uint8_t> data)
	:
		data(std::move(data)),
		width(width),
		height(height)
	{
		if(this->data.size() != 4 * width * height)
		{
			throw std::invalid_argument("bad image data size");
		}
		check_transparency();
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	void check_transparency()
	{
		assert(data.size() % 4 == 0);
		for(std::size_t i = 3; i < data.size(); i += 4)
		{
			if(data[i] != 255)
			{
				has_transparency = true;
				return;
			}
		}
		has_transparency = false;
	}

	std::vector<uint8_t> data;
	uint32_t width;
	uint32_t height;
	bool has_transparency;
};

image::image()
:
	pImpl(std::make_unique<impl>())
{
}

image::image(const fs::path& path)
:
	pImpl(std::make_unique<impl>(path))
{
}

image::image(const uint32_t width, const uint32_t height, std::vector<uint8_t> data)
:
	pImpl(std::make_unique<impl>(width, height, data))
{
}

image::image(const uint32_t width, const uint32_t height, const uint8_t* data)
:
	image(width, height, std::vector<uint8_t>(data, data + 4 * width * height))
{
}

image::~image()
{
}

uint32_t image::get_width() const
{
	return pImpl->width;
}

uint32_t image::get_height() const
{
	return pImpl->height;
}

const uint8_t* image::get_data() const
{
	return pImpl->data.data();
}

bool image::has_transparency() const
{
	return pImpl->has_transparency;
}

void image::write(const fs::path& path) const
{
	if(path.extension() == ".png")
	{
		write_png(path, pImpl->data, pImpl->width, pImpl->height);
	}
	else if(path.extension() == ".raw")
	{
		write_raw(path, pImpl->data);
	}
	else
	{
		throw std::invalid_argument("unknown image format: " + path.extension().u8string());
	}
}

static string png_color_type_name(const int color_type)
{
	switch(color_type)
	{
		case PNG_COLOR_TYPE_GRAY      : return "GRAY";
		case PNG_COLOR_TYPE_PALETTE   : return "PALETTE";
		case PNG_COLOR_TYPE_RGB       : return "RGB";
		case PNG_COLOR_TYPE_RGBA      : return "RGBA";
		case PNG_COLOR_TYPE_GRAY_ALPHA: return "GRAY_ALPHA";
	}
	return std::to_string(color_type);
}

struct png_read
{
	png_struct* png;
	png_info* info;

	png_read(const fs::path& path, std::FILE* file)
	{
		std::array<png_byte, 8> header;
		const std::size_t sig_size = std::fread(header.data(), sizeof(png_byte), header.size(), file);
		if(sig_size != header.size() || png_sig_cmp(header.data(), 0, header.size()))
		{
			throw std::runtime_error("not a PNG file: " + path.u8string());
		}

		png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(png == nullptr)
		{
			throw std::runtime_error("png_create_read_struct returned null");
		}

		info = png_create_info_struct(png);
		if(info == nullptr)
		{
			throw std::runtime_error("png_create_info_struct returned null");
		}

		if(setjmp(png_jmpbuf(png)))
		{
			throw std::runtime_error("error during png_init_io");
		}
		png_init_io(png, file);

		png_set_sig_bytes(png, static_cast<int>(header.size()));
		png_read_info(png, info);
	}

	png_read(png_read&&) = delete;
	png_read(const png_read&) = delete;
	png_read& operator=(png_read&&) = delete;
	png_read& operator=(const png_read&) = delete;

	~png_read()
	{
		png_destroy_read_struct(&png, &info, nullptr);
	}
};

std::tuple<std::vector<uint8_t>, uint32_t, uint32_t> read_png
(
	const fs::path& path
)
{
	auto file = unique_ptr<std::FILE, decltype(&std::fclose)>(std::fopen(path.string().c_str(), "rb"), &std::fclose);
	if(file == nullptr)
	{
		throw std::runtime_error("unable to read " + path.u8string());
	}

	png_read read(path, file.get());

	const uint32_t width = png_get_image_width(read.png, read.info);
	const uint32_t height = png_get_image_height(read.png, read.info);
	const auto color_type = png_get_color_type(read.png, read.info);
	const auto bit_depth = png_get_bit_depth(read.png, read.info);

	if(bit_depth == 16)
	{
	#if PNG_LIBPNG_VER >= 10504
		png_set_scale_16(read.png);
	#else
		png_set_strip_16(read.png);
	#endif
	}
	if(color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(read.png);
	}
	const bool has_tRNS = png_get_valid(read.png, read.info, PNG_INFO_tRNS);
	if(has_tRNS)
	{
		png_set_tRNS_to_alpha(read.png);
	}
	if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		png_set_expand_gray_1_2_4_to_8(read.png);
	}
	if(color_type == PNG_COLOR_TYPE_RGB || (color_type == PNG_COLOR_TYPE_PALETTE && !has_tRNS))
	{
		png_set_filler(read.png, 0xFF, PNG_FILLER_AFTER);
	}

	const auto number_of_passes = png_set_interlace_handling(read.png); // is this needed for reading?
	png_read_update_info(read.png, read.info);

	if(setjmp(png_jmpbuf(read.png)))
	{
		throw std::runtime_error("error while reading PNG file " + path.u8string());
	}

	const std::size_t rowbytes = png_get_rowbytes(read.png, read.info);

	LOG(DEBUG) << path.u8string() << ": " << width << "×" << height
			   << "; bit depth = " << +bit_depth
			   << "; color type = " << png_color_type_name(color_type)
			   << "; pass count = " << number_of_passes
			   << "; rowbytes = " << rowbytes
			   << '\n'
			;

	std::vector<uint8_t> data(height * rowbytes);
	for(std::size_t y = height; y > 0; --y)
	{
		png_read_row(read.png, &data[(y - 1) * rowbytes], nullptr);
	}
	return {std::move(data), width, height};
}

struct png_write
{
	png_struct* png;
	png_info* info;

	png_write(std::FILE* file)
	{
		png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if(png == nullptr)
		{
			throw std::runtime_error("png_create_write_struct returned null");
		}

		info = png_create_info_struct(png);
		if(info == nullptr)
		{
			throw std::runtime_error("png_create_info_struct returned null");
		}

		if(setjmp(png_jmpbuf(png)))
		{
			throw std::runtime_error("error during png_init_io");
		}
		png_init_io(png, file);
	}

	~png_write()
	{
		png_free_data(png, info, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&png, &info);
	}
};

void write_png
(
	const fs::path& path,
	const std::vector<uint8_t>& data,
	const uint32_t width,
	const uint32_t height
)
{
	// http://www.libpng.org/pub/png/spec/iso/index-object.html#3PNGfourByteUnSignedInteger
	// http://www.libpng.org/pub/png/spec/iso/index-object.html#11IHDR
	if(width == 0 || width > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
	{
		throw std::invalid_argument("width out of range");
	}
	if(height == 0 || height > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))
	{
		throw std::invalid_argument("height out of range");
	}

	assert(data.size() == 4 * width * height);

	auto file = unique_ptr<std::FILE, decltype(&std::fclose)>(std::fopen(path.string().c_str(), "wb"), &std::fclose);
	if(file == nullptr)
	{
		throw std::runtime_error(string("error opening png file for writing: ") + std::strerror(errno));
	}

	png_write write(file.get());

	const int bit_depth = 8;
	png_set_IHDR(write.png, write.info, width, height, bit_depth, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(write.png, write.info);
	const std::size_t rowsize = png_get_rowbytes(write.png, write.info);
	for(std::size_t y = height; y > 0; --y)
	{
		png_write_row(write.png, const_cast<uint8_t*>(data.data() + (y - 1) * rowsize));
	}
	png_write_end(write.png, write.info);
}

void write_raw
(
	const fs::path& path,
	const std::vector<uint8_t>& data
)
{
	assert(data.size() <= std::numeric_limits<std::streamsize>::max());
	std::ofstream f(path, std::ofstream::binary);
	f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

}
