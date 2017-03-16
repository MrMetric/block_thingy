#include "BlockRegistry.hpp"

#include <stdexcept>
#include <vector>

#include <easylogging++/easylogging++.hpp>

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
:
	max_extid(static_cast<BlockTypeExternal>(0))
{
}

unique_ptr<Base> BlockRegistry::make(const BlockType t) const
{
	const auto i = map.find(t);
	if(i == map.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<block_type_id_t>(t)));
	}
	return i->second->make(t);
}

unique_ptr<Base> BlockRegistry::make(const BlockTypeExternal te) const
{
	const auto i = extid_to_strid.find(te);
	if(i == extid_to_strid.cend())
	{
		throw std::runtime_error("invalid external block ID: " + std::to_string(static_cast<block_type_id_t>(te)));
	}
	const auto i2 = strid_to_id.find(i->second);
	if(i2 == strid_to_id.cend())
	{
		static std::vector<string> warning_for;
		if(std::find(warning_for.cbegin(), warning_for.cend(), i->second) == warning_for.cend())
		{
			LOG(WARNING) << "invalid block type in extid map: " << i->second;
			warning_for.push_back(i->second);
		}
		return make(BlockType::unknown);
	}
	const BlockType t = i2->second;
	return make(t);
}

unique_ptr<Base> BlockRegistry::make(const string& strid) const
{
	const auto i = strid_to_id.find(strid);
	if(i == strid_to_id.cend())
	{
		throw std::runtime_error("unknown block type: " + strid);
	}
	return make(i->second);
}

unique_ptr<Base> BlockRegistry::make(const Base& block) const
{
	unique_ptr<Base> new_block = make(block.type());
	*new_block = block;
	return new_block;
}

string BlockRegistry::get_strid(const BlockType t) const
{
	const auto i = id_to_strid.find(t);
	if(i == id_to_strid.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<block_type_id_t>(t)));
	}
	return i->second;
}

BlockType BlockRegistry::get_id(const string& strid) const
{
	const auto i = strid_to_id.find(strid);
	if(i == strid_to_id.cend())
	{
		throw std::runtime_error("unknown block type: " + strid);
	}
	return i->second;
}

BlockTypeExternal BlockRegistry::get_extid(const BlockType t) const
{
	const std::string strid = get_strid(t);
	const auto i = strid_to_extid.find(strid);
	if(i == strid_to_extid.cend())
	{
		throw std::runtime_error("BUG: block type " + strid + " not found in strid_to_extid");
	}
	return i->second;
}

void BlockRegistry::reset_extid_map()
{
	extid_to_strid.clear();
	for(const auto& p : id_to_strid)
	{
		extid_to_strid.emplace(static_cast<BlockTypeExternal>(p.first), p.second);
	}
	make_strid_to_extid_map();
}

static BlockTypeExternal get_max_extid
(
	const BlockRegistry::extid_map_t& extid_to_strid
)
{
	BlockTypeExternal max_extid = static_cast<BlockTypeExternal>(0);
	for(const auto& p : extid_to_strid)
	{
		if(p.first > max_extid)
		{
			max_extid = p.first;
		}
	}
	return max_extid;
}

void BlockRegistry::set_extid_map(extid_map_t map)
{
	extid_to_strid = std::move(map);
	make_strid_to_extid_map();

	// this is not necessary (yet)
	max_extid = get_max_extid(extid_to_strid);
	for(const auto& p : strid_to_id)
	{
		const string& strid = p.first;
		if(strid_to_extid.find(strid) == strid_to_extid.cend())
		{
			max_extid = static_cast<BlockTypeExternal>(static_cast<block_type_id_t>(max_extid) + 1);
			extid_to_strid.emplace(max_extid, strid);
			strid_to_extid.emplace(strid, max_extid);
		}
	}
}

const BlockRegistry::extid_map_t& BlockRegistry::get_extid_map() const
{
	return extid_to_strid;
}

void BlockRegistry::make_strid_to_extid_map()
{
	strid_to_extid.clear();
	for(const auto& p : extid_to_strid)
	{
		strid_to_extid.emplace(p.second, p.first);
	}
}

void BlockRegistry::add(const string& strid, const BlockType t)
{
	const auto i = strid_to_id.find(strid);
	if(i != strid_to_id.cend())
	{
		throw std::runtime_error("duplicate block type: " + strid);
	}
	strid_to_id.emplace(strid, t);
	id_to_strid.emplace(t, strid);

	// this might not be needed later
	if(strid_to_extid.find(strid) == strid_to_extid.cend())
	{
		max_extid = static_cast<BlockTypeExternal>(static_cast<block_type_id_t>(max_extid) + 1);
		extid_to_strid.emplace(max_extid, strid);
		strid_to_extid.emplace(strid, max_extid);
	}
}

}
