#pragma once

#include <map>
#include <memory>
#include <string>

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
	virtual std::unique_ptr<Base> make(Enum::Type) const;
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
public:
	std::unique_ptr<Base> make(const Enum::Type t) const override
	{
		return std::make_unique<T>(t);
	}
};

class BlockRegistry
{
public:
	BlockRegistry();

	template<typename T = Base>
	Enum::Type add(const std::string& strid)
	{
		return add(strid, std::make_unique<BlockMakerInstance<T>>());
	}

	std::unique_ptr<Base> make(Enum::Type) const;
	std::unique_ptr<Base> make(Enum::TypeExternal) const;
	std::unique_ptr<Base> make(const std::string& strid) const;
	std::unique_ptr<Base> make(const Base&) const;

	std::string get_strid(Enum::Type) const;
	Enum::Type get_id(const std::string& strid) const;
	Enum::TypeExternal get_extid(Enum::Type) const;

	using extid_map_t = std::map<Enum::TypeExternal, std::string>;
	void reset_extid_map();
	void set_extid_map(extid_map_t);
	const extid_map_t& get_extid_map() const;

	Enum::Type_t get_max_id() const;

	fs::path texture(Enum::Type, Block::Enum::Face);

private:
	void make_strid_to_extid_map();

	std::map<Enum::Type, std::unique_ptr<BlockMaker>> block_makers;

	std::map<std::string, Enum::Type> strid_to_id;
	std::map<Enum::Type, std::string> id_to_strid;
	extid_map_t extid_to_strid;
	std::map<std::string, Enum::TypeExternal> strid_to_extid;
	Enum::TypeExternal max_extid;

	Enum::Type add(const std::string& strid, std::unique_ptr<BlockMaker>);
};

} // namespace Block
