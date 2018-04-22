#pragma once

#include <memory>
#include <stdint.h>
#include <vector>

#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::graphics {

class image
{
public:
	image();
	explicit image(const fs::path&);
	image(uint32_t width, uint32_t height, std::vector<uint8_t> data);
	image(uint32_t width, uint32_t height, const uint8_t* data);
	~image();

	const uint8_t* get_data() const;
	uint32_t get_width() const;
	uint32_t get_height() const;
	bool has_transparency() const;

	void write(const fs::path&) const;

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
