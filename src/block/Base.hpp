#pragma once

#include <array>
#include <stdint.h>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "fwd/Game.hpp"
#include "fwd/Player.hpp"
#include "fwd/World.hpp"
#include "fwd/block/Enum/Face.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "graphics/Color.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "fwd/storage/Interface.hpp"
#include "util/filesystem.hpp"

namespace Block {

class Base
{
public:
	Base();
	Base(Enum::Type);
	Base(Enum::Type, Enum::VisibilityType);
	Base(Enum::Type, Enum::VisibilityType, const fs::path& shader);
	Base(Enum::Type, Enum::VisibilityType, const std::array<fs::path, 6>& shaders);
	virtual ~Base();

	Base(const Base&);
	virtual Base& operator=(const Base&);

	Enum::Type type() const
	{
		return type_;
	}
	virtual std::string name() const;

	/**
	 * If non-zero, this block emits light of this color
	 */
	Graphics::Color light() const
	{
		return light_;
	}
	virtual void light(const Graphics::Color&);

	fs::path shader(Enum::Face) const;
	fs::path texture(Enum::Face) const;

	glm::tvec3<uint8_t> rotation() const;
	virtual uint8_t rotation(Enum::Face) const;
	virtual void rotate_around(Enum::Face, int8_t direction);

	virtual double bounciness() const;

	/**
	 * The selection color when this block is selected
	 */
	virtual glm::dvec4 selection_color() const;

	virtual Graphics::Color light_filter() const;

	Enum::VisibilityType visibility_type() const
	{
		return visibility_type_;
	}
	bool is_opaque() const
	{
		return visibility_type_ == Enum::VisibilityType::opaque;
	}
	bool is_translucent() const
	{
		return visibility_type_ == Enum::VisibilityType::translucent;
	}
	bool is_invisible() const
	{
		return visibility_type_ == Enum::VisibilityType::invisible;
	}

	/**
	 * Can entities collide with this block?
	 */
	virtual bool is_solid() const;

	/**
	 * Can players interact with this block?
	 */
	virtual bool is_selectable() const;

	/**
	 * Is this block overwritable when attempting to place the given block at its position?
	 */
	virtual bool is_replaceable_by(const Base&) const;

	virtual void use_start
	(
		Game&,
		World&,
		Player&,
		const Position::BlockInWorld&,
		Enum::Face
	);

	virtual void save(Storage::OutputInterface&) const;
	virtual void load(Storage::InputInterface&);

protected:
	Enum::VisibilityType visibility_type_;
	Graphics::Color light_;
	std::array<fs::path, 6> shader_;
	std::array<fs::path, 6> texture_;
	glm::tvec3<uint8_t> rotation_;

private:
	Enum::Type type_;
};

}
