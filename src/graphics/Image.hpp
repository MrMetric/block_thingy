#pragma once

#include <memory>
#include <experimental/propagate_const>
#include <stdint.h>
#include <vector>

#include "util/filesystem.hpp"

namespace Graphics {

class Image
{
	public:
		Image();
		Image(const fs::path&);
		Image(uint32_t width, uint32_t height, std::vector<uint8_t> data);
		~Image();

		const uint8_t* get_data() const;
		uint32_t get_width() const;
		uint32_t get_height() const;
		bool has_transparency() const;

		void write(const fs::path&) const;

	private:
		struct impl;
		std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};

} // namespace Graphics
