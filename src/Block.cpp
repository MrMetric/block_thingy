#include "Block.hpp"

#include <Poco/BinaryReader.h>
using Poco::BinaryReader;
#include <Poco/BinaryWriter.h>
using Poco::BinaryWriter;

#include "BlockType.hpp"

Block::Block() : type_(BlockType::none) {}

Block::Block(const block_type_id_t type_id)
	:
	type_(static_cast<BlockType>(type_id))
{
}

Block::Block(const BlockType type)
	:
	type_(type)
{
}

Block::Block(BinaryReader& reader)
{
	deserialize(reader);
}

block_type_id_t Block::type_id() const
{
	return static_cast<block_type_id_t>(type_);
}

BlockType Block::type() const
{
	return type_;
}

bool Block::is_invisible() const
{
	return type_ == BlockType::none || type_ == BlockType::air;
}

bool Block::is_solid() const
{
	return type_ != BlockType::air;
}

void Block::serialize(BinaryWriter& writer)
{
	writer << type_id();
}

void Block::deserialize(BinaryReader& reader)
{
	block_type_id_t type_id_;
	reader >> type_id_;
	type_ = static_cast<BlockType>(type_id_);
}
