#pragma once

#include <array>
#include <stdint.h>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "fwd/game.hpp"
#include "fwd/Player.hpp"
#include "fwd/block/enums/Face.hpp"
#include "fwd/block/enums/type.hpp"
#include "block/enums/visibility_type.hpp"
#include "graphics/color.hpp"
#include "fwd/position/block_in_world.hpp"
#include "fwd/storage/Interface.hpp"
#include "util/filesystem.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::block {

class base
{
public:
	base();
	base(enums::type);
	base(enums::type, enums::visibility_type);
	base(enums::type, enums::visibility_type, const fs::path& shader);
	base(enums::type, enums::visibility_type, const std::array<fs::path, 6>& shaders);
	virtual ~base();

	base(const base&);
	virtual base& operator=(const base&);

	enums::type type() const
	{
		return type_;
	}
	virtual std::string name() const;

	/**
	 * If non-zero, this block emits light of this color
	 */
	graphics::color light() const
	{
		return light_;
	}
	virtual void light(const graphics::color&);

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

	virtual graphics::color light_filter() const;

	enums::visibility_type visibility_type() const
	{
		return visibility_type_;
	}
	bool is_opaque() const
	{
		return visibility_type_ == enums::visibility_type::opaque;
	}
	bool is_translucent() const
	{
		return visibility_type_ == enums::visibility_type::translucent;
	}
	bool is_invisible() const
	{
		return visibility_type_ == enums::visibility_type::invisible;
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
	virtual bool is_replaceable_by(const base&) const;

	virtual void use_start
	(
		game&,
		world::world&,
		Player&,
		const position::block_in_world&,
		enums::Face
	);

	virtual void save(storage::OutputInterface&) const;
	virtual void load(storage::InputInterface&);

protected:
	enums::visibility_type visibility_type_;
	graphics::color light_;
	std::array<fs::path, 6> shader_;
	std::array<fs::path, 6> texture_;
	glm::tvec3<uint8_t> rotation_;

private:
	enums::type type_;
};

}
