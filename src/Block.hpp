#pragma once

#include "BlockType.hpp"

class Block
{
	public:
		Block();
		explicit Block(block_type_id_t);
		explicit Block(BlockType);

		block_type_id_t type_id() const;
		BlockType type() const;
		bool is_invisible() const;
		bool is_solid() const;

	private:
		BlockType type_;
};
