#pragma once

#include <map>
#include <memory>
#include <string>

#include "fwd/block/Base.hpp"
#include "fwd/block/BlockType.hpp"

#include "std_make_unique.hpp"

namespace Block {

class BlockMaker
{
	public:
		BlockMaker();
		BlockMaker(const BlockMaker&);
		virtual ~BlockMaker();
		virtual std::unique_ptr<Base> make(BlockType) const;
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
	public:
		std::unique_ptr<Base> make(BlockType t) const override
		{
			return std::make_unique<T>(t);
		}
};

class BlockRegistry
{
	public:
		BlockRegistry();

		template<typename T = Base>
		BlockType add(const std::string& name)
		{
			return add(name, std::make_unique<BlockMakerInstance<T>>());
		}

		std::unique_ptr<Base> make(BlockType) const;
		std::unique_ptr<Base> make(BlockTypeExternal) const;
		std::unique_ptr<Base> make(const std::string& name) const;
		std::unique_ptr<Base> make(const Base&) const;

		std::string get_strid(BlockType) const;
		BlockType get_id(const std::string&) const;
		BlockTypeExternal get_extid(BlockType) const;

		using extid_map_t = std::map<BlockTypeExternal, std::string>;
		void reset_extid_map();
		void set_extid_map(extid_map_t);
		const extid_map_t& get_extid_map() const;

		block_type_id_t get_max_id() const;

	private:
		void make_strid_to_extid_map();

		std::map<BlockType, std::unique_ptr<BlockMaker>> block_makers;

		std::map<std::string, BlockType> strid_to_id;
		std::map<BlockType, std::string> id_to_strid;
		extid_map_t extid_to_strid;
		std::map<std::string, BlockTypeExternal> strid_to_extid;
		BlockTypeExternal max_extid;

		BlockType add(const std::string& name, std::unique_ptr<BlockMaker>);
};

} // namespace Block
