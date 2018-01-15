#include "test.hpp"

#include <glm/vec3.hpp>
#include <glm/gtx/color_space.hpp>

#include "game.hpp"
#include "block/enums/visibility_type.hpp"

namespace block_thingy::block {

test::test(const enums::type t)
:
	base(t, enums::visibility_type::opaque, "test")
{
}

double test::bounciness() const
{
	return 1;
}

glm::dvec4 test::selection_color() const
{
	const auto t = game::instance->get_global_ticks();
	return {glm::rgbColor(glm::dvec3(t % 360, 1, 1)), 1};
}

}
