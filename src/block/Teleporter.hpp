#include "Base.hpp"

namespace Block {

class Teleporter : public Base
{
	public:
		Teleporter(BlockType);

		BlockVisibilityType visibility_type() const override;
		bool is_solid() const override;
};

} // namespace Block
