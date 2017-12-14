#include "BlockRegistry.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

#include "language.hpp"
#include "block/Air.hpp"
#include "block/Base.hpp"
#include "block/Light.hpp"
#include "block/None.hpp"
#include "block/Teleporter.hpp"
#include "block/Test.hpp"
#include "block/Unknown.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/Type.hpp"
#include "util/logger.hpp"

using std::shared_ptr;
using std::string;
using std::unique_ptr;

namespace block_thingy::block {

using enums::Face;

BlockMaker::BlockMaker()
{
}

BlockMaker::BlockMaker(const BlockMaker&)
{
}

BlockMaker::~BlockMaker()
{
}

shared_ptr<Base> BlockMaker::make(const enums::Type) const
{
	return nullptr;
}

BlockRegistry::BlockRegistry()
:
	max_extid(static_cast<enums::TypeExternal>(0))
{
	// these must be added first (in this order!) to get the correct IDs
	add<None>("none");
	add<Air>("air");
	add<Unknown>("unknown");

	add<Test>("test");
	add<Teleporter>("teleporter");
	add<Light>("light");
}

shared_ptr<Base> BlockRegistry::get_default(const enums::Type t) const
{
	std::lock_guard<std::mutex> g(default_blocks_mutex);
	const auto i = default_blocks.find(t);
	if(i == default_blocks.cend())
	{
		throw std::runtime_error("no default block for ID: " + std::to_string(static_cast<enums::Type_t>(t)));
	}
	return i->second;
}

shared_ptr<Base> BlockRegistry::get_default(const enums::TypeExternal te) const
{
	const string strid = get_strid(te);
	if(strid_to_id.count(strid) == 0)
	{
		// TODO: should I put this in a map (like default_blocks)?
		return std::make_unique<Unknown>(enums::Type::unknown, strid);
	}
	return get_default(get_id(strid));
}

shared_ptr<Base> BlockRegistry::get_default(const string& strid) const
{
	return get_default(get_id(strid));
}

shared_ptr<Base> BlockRegistry::make(const enums::Type t) const
{
	const auto i = block_makers.find(t);
	if(i == block_makers.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<enums::Type_t>(t)));
	}
	return i->second->make(t);
}

shared_ptr<Base> BlockRegistry::make(const enums::TypeExternal te) const
{
	const string strid = get_strid(te);
	const auto i2 = strid_to_id.find(strid);
	if(i2 == strid_to_id.cend())
	{
		{
			static std::mutex m;
			std::lock_guard<std::mutex> g(m);

			static std::vector<string> warning_for;
			if(std::find(warning_for.cbegin(), warning_for.cend(), strid) == warning_for.cend())
			{
				LOG(WARN) << "invalid block type in extid map: " << strid << '\n';
				warning_for.push_back(strid);
			}
		}

		// templating and virtual functions are not combinable, so I can not do this:
		//return make(enums::Type::unknown, strid);

		return std::make_unique<Unknown>(enums::Type::unknown, strid);
	}
	const enums::Type t = i2->second;
	return make(t);
}

shared_ptr<Base> BlockRegistry::make(const string& strid) const
{
	return make(get_id(strid));
}

shared_ptr<Base> BlockRegistry::make(const shared_ptr<Base> block) const
{
	shared_ptr<Base> new_block = make(block->type());
	*new_block = *block;
	return new_block;
}

enums::Type BlockRegistry::get_id(const string& strid) const
{
	const auto i = strid_to_id.find(strid);
	if(i == strid_to_id.cend())
	{
		throw std::runtime_error("unknown block type: " + strid);
	}
	return i->second;
}

string BlockRegistry::get_strid(const enums::Type t) const
{
	const auto i = id_to_strid.find(t);
	if(i == id_to_strid.cend())
	{
		throw std::runtime_error("unknown block ID: " + std::to_string(static_cast<enums::Type_t>(t)));
	}
	return i->second;
}

string BlockRegistry::get_strid(const enums::TypeExternal te) const
{
	const auto i = extid_to_strid.find(te);
	if(i == extid_to_strid.cend())
	{
		throw std::runtime_error("invalid external block ID: " + std::to_string(static_cast<enums::Type_t>(te)));
	}
	return i->second;
}

string BlockRegistry::get_name(const enums::Type t) const
{
	return language::get("block." + get_strid(t));
}

string BlockRegistry::get_name(const string& strid) const
{
	// TODO
	return strid;
}

enums::TypeExternal BlockRegistry::get_extid(const enums::Type t) const
{
	const string strid = get_strid(t);
	const auto i = strid_to_extid.find(strid);
	if(i == strid_to_extid.cend())
	{
		LOG(BUG) << "block type " << strid << " not found in strid_to_extid\n";
		throw std::runtime_error("block type " + strid + " not found in strid_to_extid");
	}
	return i->second;
}

void BlockRegistry::reset_extid_map()
{
	extid_to_strid.clear();
	for(const auto& p : id_to_strid)
	{
		extid_to_strid.emplace(static_cast<enums::TypeExternal>(p.first), p.second);
	}
	make_strid_to_extid_map();
}

static enums::TypeExternal get_max_extid
(
	const BlockRegistry::extid_map_t& extid_to_strid
)
{
	enums::TypeExternal max_extid = static_cast<enums::TypeExternal>(0);
	for(const auto& p : extid_to_strid)
	{
		if(p.first > max_extid)
		{
			max_extid = p.first;
		}
	}
	return static_cast<enums::TypeExternal>(static_cast<enums::Type_t>(max_extid) + 1);
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
			extid_to_strid.emplace(max_extid, strid);
			strid_to_extid.emplace(strid, max_extid);
			max_extid = static_cast<enums::TypeExternal>(static_cast<enums::Type_t>(max_extid) + 1);
		}
	}
}

const BlockRegistry::extid_map_t& BlockRegistry::get_extid_map() const
{
	return extid_to_strid;
}

enums::Type_t BlockRegistry::get_max_id() const
{
	assert(block_makers.size() <= std::numeric_limits<enums::Type_t>::max());
	return static_cast<enums::Type_t>(block_makers.size());
}

void BlockRegistry::make_strid_to_extid_map()
{
	strid_to_extid.clear();
	for(const auto& p : extid_to_strid)
	{
		strid_to_extid.emplace(p.second, p.first);
	}
}

enums::Type BlockRegistry::add_
(
	const string& strid,
	unique_ptr<BlockMaker> maker
)
{
	const enums::Type t = static_cast<enums::Type>(get_max_id());
	block_makers.emplace(t, std::move(maker));

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
		extid_to_strid.emplace(max_extid, strid);
		strid_to_extid.emplace(strid, max_extid);
		max_extid = static_cast<enums::TypeExternal>(static_cast<enums::Type_t>(max_extid) + 1);
	}

	return t;
}

}
