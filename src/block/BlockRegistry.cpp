#include "BlockRegistry.hpp"

#include <stdexcept>

#include "block/Base.hpp"
#include "block/BlockType.hpp"

using std::string;
using std::unique_ptr;

namespace Block {

BlockMaker::BlockMaker()
{
}

BlockMaker::BlockMaker(const BlockMaker&)
{
}

BlockMaker::~BlockMaker()
{
}

unique_ptr<Base> BlockMaker::make(BlockType)
{
	return nullptr;
}

BlockRegistry::BlockRegistry()
{
}

unique_ptr<Base> BlockRegistry::make(const BlockType t)
{
	const auto i = map.find(t);
	if(i == map.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<block_type_id_t>(t)));
	}
	return i->second->make(t);
}

unique_ptr<Base> BlockRegistry::make(const string& name)
{
	const auto i = name_to_id.find(name);
	if(i == name_to_id.cend())
	{
		throw std::runtime_error("unknown block name: " + name);
	}
	return make(i->second);
}

unique_ptr<Base> BlockRegistry::make(const Base& block)
{
	unique_ptr<Base> new_block = make(block.type());
	*new_block = block;
	return new_block;
}

string BlockRegistry::get_name(BlockType t)
{
	const auto i = id_to_name.find(t);
	if(i == id_to_name.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<block_type_id_t>(t)));
	}
	return i->second;
}

BlockType BlockRegistry::get_id(const string& name)
{
	const auto i = name_to_id.find(name);
	if(i == name_to_id.cend())
	{
		throw std::runtime_error("unknown block name: " + name);
	}
	return i->second;
}

void BlockRegistry::add(const std::string& name, const BlockType t)
{
	const auto i = name_to_id.find(name);
	if(i != name_to_id.cend())
	{
		throw std::runtime_error("duplicate block name: " + name);
	}
	name_to_id[name] = t;
	id_to_name[t] = name;
}

}
