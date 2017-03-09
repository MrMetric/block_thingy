#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "fwd/block/Base.hpp"
#include "fwd/block/BlockType.hpp"

#include "std_make_unique.hpp"

namespace Block {

static block_type_id_t MAX_ID = 0;

class BlockMaker
{
	public:
		BlockMaker();
		BlockMaker(const BlockMaker&);
		virtual ~BlockMaker();
		virtual std::unique_ptr<Base> make(BlockType);
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
	public:
		std::unique_ptr<Base> make(BlockType type) override
		{
			return std::make_unique<T>(type);
		}
};

class BlockRegistry
{
	public:
		BlockRegistry();

		template<typename T = Base>
		BlockType add(const std::string& name)
		{
			BlockType t = static_cast<BlockType>(MAX_ID++);
			map[t] = std::make_unique<BlockMakerInstance<T>>();
			add(name, t);
			return t;
		}

		std::unique_ptr<Base> make(BlockType) const;
		std::unique_ptr<Base> make(BlockTypeExternal) const;
		std::unique_ptr<Base> make(const std::string& name) const;
		std::unique_ptr<Base> make(const Base&) const;

		std::string get_strid(BlockType) const;
		BlockType get_id(const std::string&) const;
		BlockTypeExternal get_extid(BlockType) const;

		void reset_extid_map();
		void set_extid_map(std::unordered_map<BlockTypeExternal, std::string>);
		const std::unordered_map<BlockTypeExternal, std::string>& get_extid_map() const;

	private:
		void make_strid_to_extid_map();

		std::unordered_map<BlockType, std::unique_ptr<BlockMaker>> map;

		std::unordered_map<std::string, BlockType> strid_to_id;
		std::unordered_map<BlockType, std::string> id_to_strid;
		std::unordered_map<BlockTypeExternal, std::string> extid_to_strid;
		std::unordered_map<std::string, BlockTypeExternal> strid_to_extid;
		BlockTypeExternal max_extid;

		void add(const std::string& name, BlockType);
};

} // namespace Block
