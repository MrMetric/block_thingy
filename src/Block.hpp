#pragma once

#include <Poco/BinaryReader.h>
using Poco::BinaryReader;
#include <Poco/BinaryWriter.h>
using Poco::BinaryWriter;

#include "BlockType.hpp"

class Block
{
	public:
		Block();
		explicit Block(block_type_id_t);
		explicit Block(BlockType);
		explicit Block(BinaryReader&);

		block_type_id_t type_id() const;
		BlockType type() const;
		bool is_invisible() const;
		bool is_solid() const;

		void serialize(BinaryWriter&);

	private:
		BlockType type_;

		void deserialize(BinaryReader&);
};
