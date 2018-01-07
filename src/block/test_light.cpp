#include "test_light.hpp"

#include <memory>
#include <sstream>

#include "fwd/game.hpp"
#include "Player.hpp"
#include "block/enums/visibility_type.hpp"
#include "graphics/GUI/light.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/color.hpp"
#include "util/demangled_name.hpp"
#include "fwd/world/world.hpp"

using std::string;

namespace block_thingy::block {

using graphics::color;

test_light::test_light(const enums::type t)
:
	test_light(t, color(color::max))
{
}

test_light::test_light(const enums::type t, const color& light)
:
	base(t, enums::visibility_type::opaque, "light")
{
	light_ = light;
}

string test_light::name() const
{
	std::ostringstream ss;
	ss << base::name() << ' ' << light();
	return ss.str();
}

void test_light::use_start
(
	game& g,
	world::world& world,
	Player& player,
	const position::block_in_world& pos,
	const enums::Face /*face*/
)
{
	try
	{
		player.open_gui(std::make_unique<graphics::gui::light>(g, world, *this, pos));
	}
	catch(const std::exception& e)
	{
		LOG(ERROR) << "got " << util::demangled_name(e) << " while opening \"light\" GUI: " << e.what() << '\n';
	}
}

void test_light::save(storage::OutputInterface& i) const
{
	base::save(i);
	i.set("light", light_);
}

void test_light::load(storage::InputInterface& i)
{
	base::load(i);
	light_ = i.get<color>("light");
}

}
