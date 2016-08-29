#include "Test.hpp"

namespace Block {

Test::Test()
	:
	Block(BlockType::test)
{
}

double Test::bounciness() const
{
	return 1;
}

} // namespace Block
