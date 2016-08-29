#include "Block.hpp"

namespace Block {

class Teleporter : public Block
{
	public:
		Teleporter();

		BlockVisibilityType visibility_type() const override;
		bool is_solid() const override;
};

} // namespace Block
