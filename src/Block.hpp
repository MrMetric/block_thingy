#pragma once

#include <cstdint>

using block_type_id_t = uint16_t;

enum class BlockType : block_type_id_t
{
	none = 0,
	air = 1,
	test = 2,
};

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
