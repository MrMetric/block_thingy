#pragma once

#include <memory>
#include <unordered_map>

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
		virtual std::unique_ptr<Base> make();
};

template<typename T>
class BlockMakerInstance : public BlockMaker
{
	public:
		std::unique_ptr<Base> make() override
		{
			return std::make_unique<T>();
		}
};

class BlockRegistry
{
	public:
		BlockRegistry();

		template<typename T>
		void add(const BlockType t)
		{
			map[t] = std::make_unique<BlockMakerInstance<T>>();
		}

		std::unique_ptr<Base> make(BlockType);
		std::unique_ptr<Base> make(const Base&);

	private:
		std::unordered_map<BlockType, std::unique_ptr<BlockMaker>> map;
};

} // namespace Block
