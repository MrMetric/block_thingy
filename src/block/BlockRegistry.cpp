#include "BlockRegistry.hpp"

#include "block/Base.hpp"
#include "block/BlockType.hpp"

#include "block/Air.hpp"
#include "block/Light.hpp"
#include "block/None.hpp"
#include "block/Test.hpp"
#include "block/Teleporter.hpp"

#include "std_make_unique.hpp"

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

unique_ptr<Base> BlockMaker::make()
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

unique_ptr<Base> BlockRegistry::make(const BlockType t)
{
	const auto i = map.find(t);
	if(i == map.cend())
	{
		// BlockRegistry is a friend of Base, but this does not apply to std::make_unique
		return unique_ptr<Base>(new Base(t));
	}
	return i->second->make();
}

unique_ptr<Base> BlockRegistry::make(const Base& block)
{
	unique_ptr<Base> new_block = make(block.type());
	*new_block = block;
	return new_block;
}

} // namespace Block
