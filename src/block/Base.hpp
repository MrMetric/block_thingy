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

namespace block_thingy::block {

class Base
{
public:
	Base();
	Base(enums::Type);
	Base(enums::Type, enums::VisibilityType);
	Base(enums::Type, enums::VisibilityType, const fs::path& shader);
	Base(enums::Type, enums::VisibilityType, const std::array<fs::path, 6>& shaders);
	virtual ~Base();

	Base(const Base&);
	virtual Base& operator=(const Base&);

	enums::Type type() const
	{
		return type_;
	}
	virtual std::string name() const;

	/**
	 * If non-zero, this block emits light of this color
	 */
	graphics::Color light() const
	{
		return light_;
	}
	virtual void light(const graphics::Color&);

	fs::path shader(enums::Face) const;
	fs::path texture(enums::Face) const;

	glm::tvec3<uint8_t> rotation() const;
	virtual uint8_t rotation(enums::Face) const;
	virtual void rotate_around(enums::Face, int8_t direction);

	virtual double bounciness() const;

	/**
	 * The selection color when this block is selected
	 */
	virtual glm::dvec4 selection_color() const;

	virtual graphics::Color light_filter() const;

	enums::VisibilityType visibility_type() const
	{
		return visibility_type_;
	}
	bool is_opaque() const
	{
		return visibility_type_ == enums::VisibilityType::opaque;
	}
	bool is_translucent() const
	{
		return visibility_type_ == enums::VisibilityType::translucent;
	}
	bool is_invisible() const
	{
		return visibility_type_ == enums::VisibilityType::invisible;
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
		const position::BlockInWorld&,
		enums::Face
	);

	virtual void save(storage::OutputInterface&) const;
	virtual void load(storage::InputInterface&);

protected:
	enums::VisibilityType visibility_type_;
	graphics::Color light_;
	std::array<fs::path, 6> shader_;
	std::array<fs::path, 6> texture_;
	glm::tvec3<uint8_t> rotation_;

private:
	enums::Type type_;
};

}
