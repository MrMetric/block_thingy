#include "manager.hpp"

#include <algorithm>
#include <cassert>

using std::string;

namespace block_thingy::block {

manager::manager()
:
	instance_count(0)
{
	add_component(info);

	const block_t none = create();
	assert(none.index == 0 && none.generation == 0);
	set_strid(none, "none");
	info.visibility_type(none, enums::visibility_type::invisible);

	const block_t air = create();
	set_strid(air, "air");
	info.solid(air, false);
	info.selectable(air, false);
	info.visibility_type(air, enums::visibility_type::invisible);

	// TODO: selection color animation
	const block_t test = create();
	set_strid(test, "test");
	info.bounciness(test, 1);
	info.shader_path(test, "test");

	// TODO: editing
	const block_t test_light = create();
	set_strid(test_light, "test_light");
	info.shader_path(test_light, "light");
	info.light(test_light, {graphics::color::max});

	// TODO
	const block_t test_teleporter = create();
	set_strid(test_teleporter, "test_teleporter");
	info.shader_path(test_teleporter, "teleporter");
	info.light(test_teleporter, {0, 0, 4});
}

manager::~manager()
{
}

block_t manager::create()
{
	instance_count += 1;

	uint32_t i;
	if(free_indexes.size() > 1024)
	{
		i = free_indexes.front();
		free_indexes.pop_front();
	}
	else
	{
		assert(generation.size() < (1 << 24)); // 24 = index bits
		generation.push_back(0);
		i = static_cast<uint32_t>(generation.size()) - 1;
	}
	return block_t(i, generation[i]);
}

void manager::destroy(const block_t block)
{
	if(block.index == 0 && block.generation == 0)
	{
		// the none block must not be destroyed
		// log this?
		return;
	}

	assert(instance_count != 0);
	instance_count -= 1;

	const uint32_t bi = block.index;
	assert(bi < generation.size());
	++generation[bi];
	free_indexes.push_back(bi);

	if(const auto i = block_to_strid.find(block);
		i != block_to_strid.cend())
	{
		strid_to_block.erase(i->second);
		block_to_strid.erase(i);
	}
}

bool manager::exists(const block_t block) const
{
	if(block.index >= generation.size())
	{
		return false;
	}
	return generation[block.index] == block.generation;
}

block_t manager::duplicate(const block_t block)
{
	block_t new_block = create();
	for(component::base* c : components)
	{
		c->copy(block, new_block);
	}
	return new_block;
}

/*
duplicate_blocks(block_t[] blocks):
	new_blocks = blocks
	set new index/generation
	for each manager:
		manager.copy(blocks, new_blocks)
*/

std::vector<block_t> manager::duplicate(const std::vector<block_t>& in)
{
	std::vector<block_t> out(in.size());
	for(std::size_t i = 0; i < in.size(); ++i)
	{
		out[i] = create();
	}
	for(component::base* c : components)
	{
		c->copy(in.data(), out.data(), in.size());
	}
	return out;
}

std::optional<string> manager::get_strid(const block_t block) const
{
	if(const auto i = block_to_strid.find(block);
		i != block_to_strid.cend())
	{
		return i->second;
	}
	return {};
}

std::optional<block_t> manager::get_block(const string& strid) const
{
	if(const auto i = strid_to_block.find(strid);
		i != strid_to_block.cend())
	{
		return i->second;
	}
	return {};
}
void manager::set_strid(const block_t block, const string& strid)
{
	strid_to_block.insert_or_assign(strid, block);
	block_to_strid.insert_or_assign(block, strid);
}

std::optional<string> manager::get_name(const block_t block) const
{
	if(const auto i = block_to_name.find(block);
		i != block_to_name.cend())
	{
		return i->second;
	}
	if(const auto i = block_to_strid.find(block);
		i != block_to_strid.cend())
	{
		return "block." + i->second;
	}
	return {};
}
void manager::set_name(const block_t block, const string& name)
{
	block_to_name.insert_or_assign(block, name);
}

void manager::add_component(component::base& c)
{
	components.push_back(&c);
}
void manager::unadd_component(component::base& c)
{
	if(const auto i = std::find(components.cbegin(), components.cend(), &c);
		i != components.cend())
	{
		components.erase(i);
	}
	else
	{
		// TODO: debug warning (DWARN?)
	}
}

}
