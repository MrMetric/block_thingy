#pragma once

#include <stdint.h>
#include <string>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "fwd/block/Enum/Face.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "fwd/block/Enum/VisibilityType.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/position/BlockInWorld.hpp"
#include "fwd/storage/Interface.hpp"
#include "util/filesystem.hpp"

namespace Block {

class Base
{
	public:
		Base();
		Base(Enum::Type);
		virtual ~Base();

		Base(const Base&);
		virtual Base& operator=(const Base&);

		Enum::Type type() const;
		virtual std::string name() const;

		/**
		 * If non-zero, this block emits light of this color
		 */
		virtual Graphics::Color color() const;

		virtual fs::path texture(Enum::Face) const;

		glm::tvec3<uint8_t> rotation() const;
		virtual uint8_t rotation(Enum::Face) const;
		virtual void rotate_around(Enum::Face, int8_t direction);

		virtual double bounciness() const;

		/**
		 * The selection color when this block is selected
		 */
		virtual glm::dvec4 selection_color() const;

		virtual Graphics::Color light_filter() const;

		virtual Enum::VisibilityType visibility_type() const;
		bool is_opaque() const;
		bool is_translucent() const;
		bool is_invisible() const;

		/**
		 * Can entities collide with this block?
		 */
		virtual bool is_solid() const;

		/**
		 * Can players interact with this block?
		 */
		virtual bool is_selectable() const;

		/**
		 * Is this block overwritable when attempting to place a block at its position?
		 */
		virtual bool is_replaceable() const;

		virtual void use_start(const Position::BlockInWorld&, Enum::Face);

		virtual void save(Storage::OutputInterface&) const;
		virtual void load(Storage::InputInterface&);

	protected:
		glm::tvec3<uint8_t> rotation_;

	private:
		Enum::Type type_;
};

} // namespace Block
