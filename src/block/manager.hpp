#pragma once

#include <cstddef>
#include <deque>
#include <map>
#include <optional>
#include <stdint.h>
#include <vector>

#include "block/block.hpp"
#include "block/component/info.hpp"

namespace block_thingy::block {

class manager
{
public:
	manager();
	~manager();

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

	component::info info;

	// non-owning pointer
	std::vector<component::base*> components;
	void add_component(component::base&);
	void unadd_component(component::base&);

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
};

}
