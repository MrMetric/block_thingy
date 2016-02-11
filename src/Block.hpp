#pragma once

#include <cstdint>

using block_id_type = uint16_t;

enum class BlockType : block_id_type
{
	none = 0,
	air = 1,
	test = 2,
};

class Block
{
	public:
		Block();
		explicit Block(block_id_type type);
		explicit Block(BlockType type);

		block_id_type type_id() const;
		BlockType type() const;
		bool is_invisible() const;
		bool is_solid() const;

	private:
		BlockType type_;
};
