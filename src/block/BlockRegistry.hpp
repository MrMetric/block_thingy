#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "fwd/block/Base.hpp"
#include "fwd/block/Enum/Face.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "shim/make_unique.hpp"
#include "util/filesystem.hpp"

namespace Block {

class BlockMaker
{
public:
	BlockMaker();
	BlockMaker(const BlockMaker&);
	virtual ~BlockMaker();
	virtual std::shared_ptr<Base> make(Enum::Type) const;
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
public:
	std::shared_ptr<Base> make(const Enum::Type t) const override
	{
		return std::make_shared<T>(t);
	}
};

class BlockRegistry
{
public:
	BlockRegistry();

	template<typename T, typename... Args>
	Enum::Type add(const std::string& strid, Args&&... args)
	{
		Enum::Type t = add_(strid, std::make_unique<BlockMakerInstance<T>>());
		default_blocks[t] = std::make_shared<T>(t, std::forward<Args>(args)...);
		return t;
	}

	std::shared_ptr<Base> get_default(Enum::Type) const;
	std::shared_ptr<Base> get_default(Enum::TypeExternal) const;
	std::shared_ptr<Base> get_default(const std::string& strid) const;

	std::shared_ptr<Base> make(Enum::Type) const;
	std::shared_ptr<Base> make(Enum::TypeExternal) const;
	std::shared_ptr<Base> make(const std::string& strid) const;
	std::shared_ptr<Base> make(const std::shared_ptr<Base>) const;

	Enum::Type get_id(const std::string& strid) const;
	//Enum::Type get_id(Enum::TypeExternal) const;

	std::string get_strid(Enum::Type) const;
	std::string get_strid(Enum::TypeExternal) const;

	std::string get_name(Enum::Type) const;
	std::string get_name(const std::string& strid) const;

	Enum::TypeExternal get_extid(Enum::Type) const;

	using extid_map_t = std::map<Enum::TypeExternal, std::string>;
	void reset_extid_map();
	void set_extid_map(extid_map_t);
	const extid_map_t& get_extid_map() const;

	Enum::Type_t get_max_id() const;

private:
	void make_strid_to_extid_map();
	Enum::Type add_(const std::string& strid, std::unique_ptr<BlockMaker>);

	mutable std::map<Enum::Type, std::shared_ptr<Block::Base>> default_blocks;
	mutable std::mutex default_blocks_mutex;
	std::map<Enum::Type, std::unique_ptr<BlockMaker>> block_makers;

	std::map<std::string, Enum::Type> strid_to_id;
	std::map<Enum::Type, std::string> id_to_strid;
	extid_map_t extid_to_strid;
	std::map<std::string, Enum::TypeExternal> strid_to_extid;
	Enum::TypeExternal max_extid;
};

} // namespace Block
