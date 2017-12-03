#include "Base.hpp"

#include <stdexcept>
#include <string>

#include "Game.hpp"
#include "Player.hpp"
#include "block/RotationUtil.hpp"
#include "block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/Color.hpp"
#include "storage/Interface.hpp"
#include "storage/msgpack/BlockType.hpp"
#include "storage/msgpack/glm_vec3.hpp"
#include "util/logger.hpp"

using std::string;

namespace Block {

Base::Base()
:
	Base(Enum::Type::none, Enum::VisibilityType::opaque)
{
}

Base::Base(const Enum::Type type)
:
	Base(type, Enum::VisibilityType::opaque)
{
}

Base::Base(const Enum::Type type, const Enum::VisibilityType visibility_type_)
:
	Base(type, visibility_type_, "default")
{
}

Base::Base(const Enum::Type type, const Enum::VisibilityType visibility_type_, const fs::path& shader)
:
	visibility_type_(visibility_type_),
	type_(type)
{
	shader_.fill("shaders/block" / shader);
}

Base::Base(const Enum::Type type, const Enum::VisibilityType visibility_type_, const std::array<fs::path, 6>& shaders)
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
	const auto type1 = static_cast<Enum::Type_t>(type());
	const auto type2 = static_cast<Enum::Type_t>(that.type());
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

void Base::light(const Graphics::Color& light)
{
	light_ = light;
}

fs::path Base::shader(const Enum::Face face) const
{
	return shader_[static_cast<std::size_t>(RotationUtil::rotate_face(face, rotation()))];
}

fs::path Base::texture(const Enum::Face face) const
{
	return texture_[static_cast<std::size_t>(RotationUtil::rotate_face(face, rotation()))];
}

glm::tvec3<uint8_t> Base::rotation() const
{
	return rotation_;
}

uint8_t Base::rotation(const Enum::Face face) const
{
	return RotationUtil::face_rotation_LUT.at(rotation_)[face];
}

void Base::rotate_around(const Enum::Face face, int8_t direction)
{
	auto r
	 = RotationUtil::rotate(rotation_.x, {1, 0, 0})
	 * RotationUtil::rotate(rotation_.y, {0, 1, 0})
	 * RotationUtil::rotate(rotation_.z, {0, 0, 1});

	RotationUtil::ivec3 axis(glm::uninitialize);
	switch(face)
	{
		case Enum::Face::front : direction *= +1; axis = {0, 0, 1}; break;
		case Enum::Face::back  : direction *= -1; axis = {0, 0, 1}; break;
		case Enum::Face::top   : direction *= +1; axis = {0, 1, 0}; break;
		case Enum::Face::bottom: direction *= -1; axis = {0, 1, 0}; break;
		case Enum::Face::right : direction *= +1; axis = {1, 0, 0}; break;
		case Enum::Face::left  : direction *= -1; axis = {1, 0, 0}; break;
	}
	r *= RotationUtil::rotate(direction, axis);

	rotation_ = RotationUtil::mat_to_rot(r);
}

double Base::bounciness() const
{
	return 0;
}

glm::dvec4 Base::selection_color() const
{
	return {1, 1, 1, 1};
}

Graphics::Color Base::light_filter() const
{
	return {Graphics::Color::max};
}

bool Base::is_solid() const
{
	return true;
}

bool Base::is_selectable() const
{
	return true;
}

bool Base::is_replaceable_by(const Base&) const
{
	return false;
}

void Base::use_start
(
	Game&,
	World&,
	Player& player,
	const Position::BlockInWorld& pos,
	const Enum::Face face
)
{
	LOG(DEBUG) << "+use on block " << pos << ":" << face << " by player " << player.name << '\n';
}

void Base::save(Storage::OutputInterface& i) const
{
	Enum::Type t = (type_ == Enum::Type::none) ? Enum::Type::air : type_;
	Enum::TypeExternal te = Game::instance->block_registry.get_extid(t);
	i.set("", te);
	if(rotation_.x != 0 || rotation_.y != 0 || rotation_.z != 0)
	{
		i.set("r", rotation_);
	}
}

void Base::load(Storage::InputInterface& i)
{
	// type is set before loading
	i.maybe_get("r", rotation_);
}

}
