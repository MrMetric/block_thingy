#include "Base.hpp"

namespace Block {

class Teleporter : public Base
{
public:
	Teleporter(Enum::Type);

	Enum::VisibilityType visibility_type() const override;
	bool is_solid() const override;
};

} // namespace Block
