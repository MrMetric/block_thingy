#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "fwd/block/Base.hpp"
#include "fwd/block/Enum/Type.hpp"

namespace block_thingy::block {

class BlockMaker
{
public:
	BlockMaker();
	BlockMaker(const BlockMaker&);
	virtual ~BlockMaker();
	virtual std::shared_ptr<Base> make(enums::Type) const;
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
public:
	std::shared_ptr<Base> make(const enums::Type t) const override
	{
		return std::make_shared<T>(t);
	}
};

class BlockRegistry
{
public:
	BlockRegistry();

	BlockRegistry(BlockRegistry&&) = delete;
	BlockRegistry(const BlockRegistry&) = delete;
	BlockRegistry& operator=(BlockRegistry&&) = delete;
	BlockRegistry& operator=(const BlockRegistry&) = delete;

	template<typename T, typename... Args>
	enums::Type add(const std::string& strid, Args&&... args)
	{
		enums::Type t = add_(strid, std::make_unique<BlockMakerInstance<T>>());
		default_blocks[t] = std::make_shared<T>(t, std::forward<Args>(args)...);
		return t;
	}

	std::shared_ptr<Base> get_default(enums::Type) const;
	std::shared_ptr<Base> get_default(enums::TypeExternal) const;
	std::shared_ptr<Base> get_default(const std::string& strid) const;

	std::shared_ptr<Base> make(enums::Type) const;
	std::shared_ptr<Base> make(enums::TypeExternal) const;
	std::shared_ptr<Base> make(const std::string& strid) const;
	std::shared_ptr<Base> make(const std::shared_ptr<Base>) const;

	enums::Type get_id(const std::string& strid) const;
	//enums::Type get_id(enums::TypeExternal) const;

	std::string get_strid(enums::Type) const;
	std::string get_strid(enums::TypeExternal) const;

	std::string get_name(enums::Type) const;
	std::string get_name(const std::string& strid) const;

	enums::TypeExternal get_extid(enums::Type) const;

	using extid_map_t = std::map<enums::TypeExternal, std::string>;
	void reset_extid_map();
	void set_extid_map(extid_map_t);
	const extid_map_t& get_extid_map() const;

	enums::Type_t get_max_id() const;

private:
	void make_strid_to_extid_map();
	enums::Type add_(const std::string& strid, std::unique_ptr<BlockMaker>);

	mutable std::map<enums::Type, std::shared_ptr<Base>> default_blocks;
	mutable std::mutex default_blocks_mutex;
	std::map<enums::Type, std::unique_ptr<BlockMaker>> block_makers;

	std::map<std::string, enums::Type> strid_to_id;
	std::map<enums::Type, std::string> id_to_strid;
	extid_map_t extid_to_strid;
	std::map<std::string, enums::TypeExternal> strid_to_extid;
	enums::TypeExternal max_extid;
};

}
