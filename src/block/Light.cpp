#include "Light.hpp"

#include <memory>
#include <sstream>

#include <glm/vec3.hpp>

#include "Game.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/GUI/Light.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/Color.hpp"

using std::string;
using Graphics::Color;

namespace Block {

Light::Light(const Enum::Type t)
:
	Light(t, Color(glm::dvec3(1, 1, 1)))
{
}

Light::Light(const Enum::Type t, const Color& light)
:
	Base(t, Enum::VisibilityType::opaque, "light")
{
	light_ = light;
}

string Light::name() const
{
	std::ostringstream ss;
	ss << "Light " << light();
	return ss.str();
}

void Light::use_start
(
	Game& game,
	World& world,
	Player& player,
	const Position::BlockInWorld& pos,
	const Enum::Face
)
{
	player.open_gui(std::make_unique<Graphics::GUI::Light>(game, world, *this, pos));
}

void Light::save(Storage::OutputInterface& i) const
{
	Base::save(i);
	i.set("light", light_);
}

void Light::load(Storage::InputInterface& i)
{
	Base::load(i);
	light_ = i.get<Color>("light");
}

}
