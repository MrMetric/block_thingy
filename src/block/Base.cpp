#include "Base.hpp"

#include <stdexcept>
#include <string>

#include "Game.hpp"
#include "Player.hpp"
#include "block/BlockRegistry.hpp"
#include "block/RotationUtil.hpp"
#include "block/Enum/Face.hpp"
#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/Color.hpp"
#include "position/BlockInWorld.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"
#include "storage/msgpack/glm_vec3.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::block {

Base::Base()
:
	Base(enums::Type::none, enums::VisibilityType::opaque)
{
}

Base::Base(const enums::Type type)
:
	Base(type, enums::VisibilityType::opaque)
{
}

Base::Base(const enums::Type type, const enums::VisibilityType visibility_type_)
:
	Base(type, visibility_type_, "default")
{
}

Base::Base(const enums::Type type, const enums::VisibilityType visibility_type_, const fs::path& shader)
:
	visibility_type_(visibility_type_),
	type_(type)
{
	shader_.fill("shaders/block" / shader);
}

Base::Base(const enums::Type type, const enums::VisibilityType visibility_type_, const std::array<fs::path, 6>& shaders)
:
	visibility_type_(visibility_type_),
	type_(type)
{
	for(std::size_t i = 0; i < 6; ++i)
	{
		shader_[i] = "shaders/block" / shaders[i];
	}
}

Base::~Base()
{
}

Base::Base(const Base& that)
:
	type_(that.type_)
{
	operator=(that);
}

Base& Base::operator=(const Base& that)
{
	const auto type1 = static_cast<enums::Type_t>(type());
	const auto type2 = static_cast<enums::Type_t>(that.type());
	if(type1 != type2)
	{
		throw std::runtime_error("can not copy " + std::to_string(type2) + " to " + std::to_string(type1));
	}

	visibility_type_ = that.visibility_type_;
	light_ = that.light_;
	shader_ = that.shader_;
	texture_ = that.texture_;
	rotation_ = that.rotation_;

	return *this;
}

string Base::name() const
{
	return Game::instance->block_registry.get_name(type());
}

void Base::light(const graphics::Color& light)
{
	light_ = light;
}

fs::path Base::shader(const enums::Face face) const
{
	return shader_[static_cast<std::size_t>(rotation_util::rotate_face(face, rotation()))];
}

fs::path Base::texture(const enums::Face face) const
{
	return texture_[static_cast<std::size_t>(rotation_util::rotate_face(face, rotation()))];
}

glm::tvec3<uint8_t> Base::rotation() const
{
	return rotation_;
}

uint8_t Base::rotation(const enums::Face face) const
{
	return rotation_util::face_rotation_LUT.at(rotation_)[face];
}

void Base::rotate_around(const enums::Face face, int8_t direction)
{
	auto r
	 = rotation_util::rotate(rotation_.x, {1, 0, 0})
	 * rotation_util::rotate(rotation_.y, {0, 1, 0})
	 * rotation_util::rotate(rotation_.z, {0, 0, 1});

	rotation_util::ivec3 axis(glm::uninitialize);
	switch(face)
	{
		case enums::Face::front : direction *= +1; axis = {0, 0, 1}; break;
		case enums::Face::back  : direction *= -1; axis = {0, 0, 1}; break;
		case enums::Face::top   : direction *= +1; axis = {0, 1, 0}; break;
		case enums::Face::bottom: direction *= -1; axis = {0, 1, 0}; break;
		case enums::Face::right : direction *= +1; axis = {1, 0, 0}; break;
		case enums::Face::left  : direction *= -1; axis = {1, 0, 0}; break;
	}
	r *= rotation_util::rotate(direction, axis);

	rotation_ = rotation_util::mat_to_rot(r);
}

double Base::bounciness() const
{
	return 0;
}

glm::dvec4 Base::selection_color() const
{
	return {1, 1, 1, 1};
}

graphics::Color Base::light_filter() const
{
	return {graphics::Color::max};
}

bool Base::is_solid() const
{
	return true;
}

bool Base::is_selectable() const
{
	return true;
}

bool Base::is_replaceable_by(const Base& /*block*/) const
{
	return false;
}

void Base::use_start
(
	Game& /*game*/,
	World& /*world*/,
	Player& player,
	const position::BlockInWorld& pos,
	const enums::Face face
)
{
	LOG(DEBUG) << "+use on block " << pos << ':' << face << " by player " << player.name << '\n';
}

void Base::save(storage::OutputInterface& i) const
{
	enums::Type t = (type_ == enums::Type::none) ? enums::Type::air : type_;
	enums::TypeExternal te = Game::instance->block_registry.get_extid(t);
	i.set("", te);
	if(rotation_.x != 0 || rotation_.y != 0 || rotation_.z != 0)
	{
		i.set("r", rotation_);
	}
}

void Base::load(storage::InputInterface& i)
{
	// type is set before loading
	i.maybe_get("r", rotation_);
}

}
