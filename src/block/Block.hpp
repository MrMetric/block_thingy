#pragma once

#include "BlockType.hpp"
#include "BlockVisibilityType.hpp"

#include "fwd/graphics/Color.hpp"

namespace Block {

class Block
{
	public:
		Block();
		explicit Block(block_type_id_t);
		explicit Block(BlockType);

		block_type_id_t type_id() const;
		BlockType type() const;

		/**
		 * If non-zero, this block emits light of this color
		 */
		Graphics::Color color() const;

		BlockVisibilityType visibility_type() const;
		bool is_opaque() const;
		bool is_translucent() const;
		bool is_invisible() const;

		/**
		 * Can entities collide with this block?
		 */
		bool is_solid() const;

		/**
		 * Can players interact with this block?
		 */
		bool is_selectable() const;

	private:
		BlockType type_;
};

} // namespace Block
