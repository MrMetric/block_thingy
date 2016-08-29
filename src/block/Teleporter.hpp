#include "Base.hpp"

namespace Block {

class Teleporter : public Base
{
	public:
		Teleporter();

		BlockVisibilityType visibility_type() const override;
		bool is_solid() const override;
};

} // namespace Block
