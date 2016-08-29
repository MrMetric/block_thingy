#include "BlockRegistry.hpp"

#include "block/Base.hpp"
#include "block/BlockType.hpp"

#include "block/Air.hpp"
#include "block/Light.hpp"
#include "block/None.hpp"
#include "block/Test.hpp"
#include "block/Teleporter.hpp"

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

std::unique_ptr<Base> BlockMaker::make()
{
	return nullptr;
}

BlockRegistry::BlockRegistry()
{
	add<Air>(BlockType::air);
	add<Light>(BlockType::light);
	add<None>(BlockType::none);
	add<Test>(BlockType::test);
	add<Teleporter>(BlockType::teleporter);
}

std::unique_ptr<Base> BlockRegistry::make(const BlockType t)
{
	const auto i = map.find(t);
	if(i == map.cend())
	{
		return std::make_unique<Base>(t);
	}
	return i->second->make();
}

std::unique_ptr<Base> BlockRegistry::make(const Base& block)
{
	std::unique_ptr<Base> new_block = make(block.type());
	*new_block = block;
	return new_block;
}

} // namespace Block
