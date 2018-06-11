#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <map>
#include <optional>
#include <stdint.h>
#include <vector>

#include "fwd/game.hpp"
#include "fwd/Player.hpp"
#include "block/block.hpp"
#include "block/component/info.hpp"
#include "fwd/block/enums/Face.hpp"
#include "fwd/position/block_in_world.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy::block {

class manager
{
public:
	manager();
	~manager();

	const block_t NONE = block_t();
	const block_t AIR = block_t(1, 0);

	block_t create();
	void destroy(block_t);
	bool exists(block_t) const;
	block_t duplicate(block_t);
	std::vector<block_t> duplicate(const std::vector<block_t>&);

	std::optional<std::string> get_strid(block_t) const;
	std::optional<block_t> get_block(const std::string& strid) const;
	void set_strid(block_t, const std::string&);

	std::optional<std::string> get_name(block_t) const;
	void set_name(block_t, const std::string&);

	std::optional<std::string> get_group(block_t) const;
	void set_group(block_t, const std::string&);

	component::info info;

	// non-owning pointer
	std::vector<component::base*> components;
	void add_component(component::base&);
	void unadd_component(component::base&);

	using block_transformer_t = std::function<
		block_t
		(
			game&,
			Player&,
			world::world&,
			const position::block_in_world&,
			block_t old_block,
			block_t new_block,
			enums::Face
		)>;

	void add_break_transformer(block_transformer_t);
	block_t process_break
	(
		game&,
		Player&,
		world::world&,
		const position::block_in_world&,
		enums::Face,
		block_t
	);

	void add_place_transformer(block_transformer_t);
	block_t process_place
	(
		game&,
		Player&,
		world::world&,
		const position::block_in_world&,
		enums::Face,
		block_t
	);

	using use_listener_t = std::function<
		void
		(
			game&,
			Player&,
			world::world&,
			const position::block_in_world&,
			enums::Face,
			block_t
		)>;

	void hook_use_start(use_listener_t);
	void start_use
	(
		game&,
		Player&,
		world::world&,
		const position::block_in_world&,
		enums::Face,
		block_t
	);

	void hook_use_end(use_listener_t);
	void end_use
	(
		game&,
		Player&,
		world::world&,
		const position::block_in_world&,
		enums::Face,
		block_t
	);

	// for msgpack
	template<typename T> void save(T&) const;
	template<typename T> void load(const T&);

private:
	std::vector<uint8_t> generation;
	std::deque<uint32_t> free_indexes;
	std::size_t instance_count;
	std::map<block_t, std::string> block_to_strid;
	std::map<std::string, block_t> strid_to_block;
	std::map<block_t, std::string> block_to_name;
	std::map<block_t, std::string> block_to_group;

	std::vector<block_transformer_t> break_transformers;
	std::vector<block_transformer_t> place_transformers;

	std::vector<use_listener_t> use_start_listeners;
	std::vector<use_listener_t> use_end_listeners;
};

}
