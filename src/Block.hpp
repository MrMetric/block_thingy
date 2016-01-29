#pragma once

#include <cstdint>

using block_id_type = uint_fast16_t;

class Block
{
	public:
		explicit Block(block_id_type type);
		~Block();

		__attribute__((pure))
		block_id_type type() const;

	private:
		block_id_type id;
};