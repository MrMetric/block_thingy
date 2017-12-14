#include "Light.hpp"

#include <memory>
#include <sstream>

#include "fwd/Game.hpp"
#include "Player.hpp"
#include "fwd/World.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/GUI/Light.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/Color.hpp"

using std::string;

namespace block_thingy::block {

using graphics::Color;

Light::Light(const enums::Type t)
:
	Light(t, Color(Color::max))
{
}

Light::Light(const enums::Type t, const Color& light)
:
	Base(t, enums::VisibilityType::opaque, "light")
{
	light_ = light;
}

string Light::name() const
{
	std::ostringstream ss;
	ss << Base::name() << ' ' << light();
	return ss.str();
}

void Light::use_start
(
	Game& game,
	World& world,
	Player& player,
	const position::BlockInWorld& pos,
	const enums::Face /*face*/
)
{
	player.open_gui(std::make_unique<graphics::gui::Light>(game, world, *this, pos));
}

void Light::save(storage::OutputInterface& i) const
{
	Base::save(i);
	i.set("light", light_);
}

void Light::load(storage::InputInterface& i)
{
	Base::load(i);
	light_ = i.get<Color>("light");
}

}
