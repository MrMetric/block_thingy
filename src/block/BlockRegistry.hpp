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

		std::unique_ptr<Base> make(BlockType);
		std::unique_ptr<Base> make(const std::string& name);
		std::unique_ptr<Base> make(const Base&);

		std::string get_name(BlockType);
		BlockType get_id(const std::string&);

	private:
		std::unordered_map<BlockType, std::unique_ptr<BlockMaker>> map;
		std::unordered_map<std::string, BlockType> name_to_id;
		std::unordered_map<BlockType, std::string> id_to_name;

		void add(const std::string& name, BlockType);
};

} // namespace Block
