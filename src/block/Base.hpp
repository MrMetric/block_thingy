#pragma once

#include <glm/vec4.hpp>

#include "block/BlockType.hpp"
#include "fwd/block/BlockVisibilityType.hpp"
#include "fwd/graphics/Color.hpp"
#include "fwd/storage/Interface.hpp"

namespace Block {

class Base
{
	public:
		Base();
		explicit Base(block_type_id_t);
		explicit Base(BlockType);
		virtual ~Base();

		Base(const Base&);
		virtual Base& operator=(const Base&);

		block_type_id_t type_id() const;
		BlockType type() const;

		/**
		 * If non-zero, this block emits light of this color
		 */
		virtual Graphics::Color color() const;

		virtual double bounciness() const;

		/**
		 * The selection color when this block is selected
		 */
		virtual glm::dvec4 selection_color() const;

		virtual BlockVisibilityType visibility_type() const;
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

		virtual void save(Storage::OutputInterface&) const;
		virtual void load(Storage::InputInterface&);

	private:
		BlockType type_;
};

} // namespace Block
