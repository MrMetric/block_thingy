#pragma once

#include <cstdint>

using block_id_type = uint16_t;

class __attribute__((__packed__)) Block
{
	public:
		Block();
		explicit Block(block_id_type type);

		block_id_type type() const;

	private:
		block_id_type id;
};
