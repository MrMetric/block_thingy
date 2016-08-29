#pragma once

#include "block/BlockType.hpp"
#include "fwd/block/BlockVisibilityType.hpp"

#include "fwd/graphics/Color.hpp"

namespace Block {

class Block
{
	public:
		Block();
		explicit Block(block_type_id_t);
		explicit Block(BlockType);
		virtual ~Block();

		Block(const Block&);
		virtual void operator=(const Block&);

		block_type_id_t type_id() const;
		BlockType type() const;

		/**
		 * If non-zero, this block emits light of this color
		 */
		virtual Graphics::Color color() const;

		virtual double bounciness() const;

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

	private:
		BlockType type_;
};

} // namespace Block
