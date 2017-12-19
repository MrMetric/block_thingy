#include "base.hpp"

#include <stdexcept>
#include <string>

#include "game.hpp"
#include "Player.hpp"
#include "block/BlockRegistry.hpp"
#include "block/rotation_util.hpp"
#include "block/enums/Face.hpp"
#include "block/enums/type.hpp"
#include "block/enums/visibility_type.hpp"
#include "graphics/color.hpp"
#include "position/block_in_world.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/block_type.hpp"
#include "storage/msgpack/glm_vec3.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::block {

base::base()
:
	base(enums::type::none, enums::visibility_type::opaque)
{
}

base::base(const enums::type type)
:
	base(type, enums::visibility_type::opaque)
{
}

base::base(const enums::type type, const enums::visibility_type visibility_type_)
:
	base(type, visibility_type_, "default")
{
}

base::base(const enums::type type, const enums::visibility_type visibility_type_, const fs::path& shader)
:
	visibility_type_(visibility_type_),
	type_(type)
{
	shader_.fill("shaders/block" / shader);
}

base::base(const enums::type type, const enums::visibility_type visibility_type_, const std::array<fs::path, 6>& shaders)
:
	visibility_type_(visibility_type_),
	type_(type)
{
	for(std::size_t i = 0; i < 6; ++i)
	{
		shader_[i] = "shaders/block" / shaders[i];
	}
}

base::~base()
{
}

base::base(const base& that)
:
	type_(that.type_)
{
	operator=(that);
}

base& base::operator=(const base& that)
{
	const auto type1 = static_cast<enums::type_t>(type());
	const auto type2 = static_cast<enums::type_t>(that.type());
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

string base::name() const
{
	return game::instance->block_registry.get_name(type());
}

void base::light(const graphics::color& light)
{
	light_ = light;
}

fs::path base::shader(const enums::Face face) const
{
	return shader_[static_cast<std::size_t>(rotation_util::rotate_face(face, rotation()))];
}

fs::path base::texture(const enums::Face face) const
{
	return texture_[static_cast<std::size_t>(rotation_util::rotate_face(face, rotation()))];
}

glm::tvec3<uint8_t> base::rotation() const
{
	return rotation_;
}

uint8_t base::rotation(const enums::Face face) const
{
	return rotation_util::face_rotation_LUT.at(rotation_)[face];
}

void base::rotate_around(const enums::Face face, int8_t direction)
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

double base::bounciness() const
{
	return 0;
}

glm::dvec4 base::selection_color() const
{
	return {1, 1, 1, 1};
}

graphics::color base::light_filter() const
{
	return {graphics::color::max};
}

bool base::is_solid() const
{
	return true;
}

bool base::is_selectable() const
{
	return true;
}

bool base::is_replaceable_by(const base& /*block*/) const
{
	return false;
}

void base::use_start
(
	game& /*g*/,
	world::world& /*world*/,
	Player& player,
	const position::block_in_world& pos,
	const enums::Face face
)
{
	LOG(DEBUG) << "+use on block " << pos << ':' << face << " by player " << player.name << '\n';
}

void base::save(storage::OutputInterface& i) const
{
	enums::type t = (type_ == enums::type::none) ? enums::type::air : type_;
	enums::type_external te = game::instance->block_registry.get_extid(t);
	i.set("", te);
	if(rotation_.x != 0 || rotation_.y != 0 || rotation_.z != 0)
	{
		i.set("r", rotation_);
	}
}

void base::load(storage::InputInterface& i)
{
	// type is set before loading
	i.maybe_get("r", rotation_);
}

}
