#include "Block.hpp"

Block::Block() : id(0) {}

Block::Block(block_id_type type)
	:
	id(type)
{
}

block_id_type Block::type() const
{
	return id;
}
