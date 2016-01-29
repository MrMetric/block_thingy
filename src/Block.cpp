#include "Block.hpp"

Block::Block(block_id_type type)
	:
	id(type)
{
}

Block::~Block()
{
}

block_id_type Block::type() const
{
	return this->id;
}