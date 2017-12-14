#include "Test.hpp"

#include <glm/gtx/color_space.hpp>
#include <glm/vec3.hpp>

#include "Game.hpp"
#include "World.hpp"
#include "block/Enum/VisibilityType.hpp"

namespace block_thingy::block {

Test::Test(const enums::Type t)
:
	Base(t, enums::VisibilityType::opaque, "test")
{
}

double Test::bounciness() const
{
	return 1;
}

glm::dvec4 Test::selection_color() const
{
	const auto t = Game::instance->world.get_ticks();
	return {glm::rgbColor(glm::dvec3(t % 360, 1, 1)), 1};
}

}
