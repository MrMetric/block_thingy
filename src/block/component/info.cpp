#include "info.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>

#include <msgpack.hpp>

#include "game.hpp"
#include "block/rotation_util.hpp"
#include "storage/msgpack_util.hpp"
#include "storage/msgpack/block.hpp"
#include "storage/msgpack/color.hpp"
#include "storage/msgpack/fs_path.hpp"
#include "storage/msgpack/glm_vec3.hpp"
#include "storage/msgpack/glm_vec4.hpp"

using std::string;

namespace block_thingy::block::component {

template<typename Key, typename Value>
Value get_value(const std::map<Key, Value>& map, const Key& key, const Value& d)
{
	if(const auto i = map.find(key);
		i != map.cend())
	{
		return i->second;
	}
	return d;
}

template<typename Key, typename Value>
void set_value(std::map<Key, Value>& map, const Key& key, const Value& v, const Value& d)
{
	if(v == d)
	{
		map.erase(key);
	}
	else
	{
		map.insert_or_assign(key, v);
	}
}

static void pack_path_map
(
	msgpack::packer<std::ofstream>& o,
	const std::map<block_t, std::array<fs::path, 6>>& map
)
{
	// pack_map takes uint32_t
	assert(map.size() <= std::numeric_limits<uint32_t>::max());
	o.pack_map(static_cast<uint32_t>(map.size()));
	for(const auto& [block, paths] : map)
	{
		o.pack(block);
		if(std::count(paths.cbegin() + 1, paths.cend(), paths[0]) == 5)
		{
			o.pack(paths[0]);
		}
		else
		{
			o.pack(paths);
		}
	}
}

static void load_path_map
(
	const std::map<string, msgpack::object>& m,
	const string& key,
	std::map<block_t, std::array<fs::path, 6>>& map
)
{
	map.clear();

	const auto m_i = m.find(key);
	if(m_i == m.cend())
	{
		return;
	}

	const msgpack::object& map_o = m_i->second;
	if(map_o.type != msgpack::type::MAP) throw msgpack::type_error();
	const auto map_om = map_o.as<std::map<block_t, msgpack::object>>();
	for(const auto& [block, o] : map_om)
	{
		if(o.type == msgpack::type::STR)
		{
			auto& paths = map.emplace(block, std::array<fs::path, 6>{}).first->second;
			paths.fill(o.as<fs::path>());
		}
		else
		{
			map.emplace(block, o.as<std::array<fs::path, 6>>());
		}
	}
}

template<typename T>
void copy_value(std::map<block_t, T>& map, const block_t in, const block_t out)
{
	if(const auto i = map.find(in);
		i != map.cend())
	{
		map.insert_or_assign(out, i->second);
	}
	else
	{
		map.erase(out);
	}
}

string info::get_id() const
{
	return "info";
}

void info::copy(const block_t in, const block_t out)
{
	copy_value(solid_          , in, out);
	copy_value(bounciness_     , in, out);
	copy_value(light_          , in, out);
	copy_value(light_filter_   , in, out);
	copy_value(rotation_       , in, out);
	copy_value(selectable_     , in, out);
	copy_value(selection_color_, in, out);
	copy_value(visibility_type_, in, out);
	copy_value(shader_path_    , in, out);
	copy_value(texture_path_   , in, out);
	copy_value(texture_info_   , in, out);
}
void info::copy(const block_t* const in, const block_t* const out, const std::size_t N)
{
	// TODO: data-oriented style
	for(std::size_t i = 0; i < N; ++i)
	{
		copy(*(in + i), *(out + i));
	}
}

void info::save(msgpack::packer<std::ofstream>& o) const
{
	o.pack_map(10);
	o.pack("solid"          ); o.pack(solid_          );
	o.pack("bounciness"     ); o.pack(bounciness_     );
	o.pack("light"          ); o.pack(light_          );
	o.pack("light_filter"   ); o.pack(light_filter_   );
	o.pack("rotation"       ); o.pack(rotation_       );
	o.pack("selectable"     ); o.pack(selectable_     );
	o.pack("selection_color"); o.pack(selection_color_);
	o.pack("visibility_type"); o.pack(visibility_type_);
	o.pack("shader_path"    ); pack_path_map(o, shader_path_);
	o.pack("texture_path"   ); pack_path_map(o, texture_path_);
	// texture_info_ is generated from texture_path_
}
void info::load(const msgpack::object& o)
{
	if(o.type != msgpack::type::MAP) throw msgpack::type_error();
	if(o.via.map.size != 10) throw msgpack::type_error();
	const auto m = o.as<std::map<string, msgpack::object>>();

	using storage::find_in_map;
	find_in_map(m, "solid"          , solid_          );
	find_in_map(m, "bounciness"     , bounciness_     );
	find_in_map(m, "light"          , light_          );
	find_in_map(m, "light_filter"   , light_filter_   );
	find_in_map(m, "rotation"       , rotation_       );
	find_in_map(m, "selectable"     , selectable_     );
	find_in_map(m, "selection_color", selection_color_);
	find_in_map(m, "visibility_type", visibility_type_);
	load_path_map(m, "shader_path"  , shader_path_    );
	load_path_map(m, "texture_path" , texture_path_   );

	for(const auto& [block, paths] : texture_path_)
	{
		const auto i = texture_info_.emplace(block, std::array<resource_manager::block_texture_info, 6>{}).first;
		for(std::size_t face_i = 0; face_i < paths.size(); ++face_i)
		{
			i->second[face_i] = game::instance->resource_manager.get_block_texture(paths[face_i]);
		}
	}
}

bool info::solid(const block_t block) const
{
	return get_value(solid_, block, true);
}
void info::solid(const block_t block, const bool value)
{
	set_value(solid_, block, value, true);
}

double info::bounciness(const block_t block) const
{
	return get_value(bounciness_, block, 0.0);
}
void info::bounciness(const block_t block, const double value)
{
	set_value(bounciness_, block, value, 0.0);
}

graphics::color info::light(const block_t block) const
{
	return get_value(light_, block, {0, 0, 0});
}
void info::light(const block_t block, const graphics::color& value)
{
	set_value(light_, block, value, {0, 0, 0});
}

graphics::color info::light_filter(const block_t block) const
{
	return get_value(light_filter_, block, {graphics::color::max});
}
void info::light_filter(const block_t block, const graphics::color& value)
{
	set_value(light_filter_, block, value, {graphics::color::max});
}

glm::tvec3<uint8_t> info::rotation(const block_t block) const
{
	return get_value(rotation_, block, {0, 0, 0});
}
void info::rotation(const block_t block, const glm::tvec3<uint8_t>& value)
{
	set_value(rotation_, block, value, {0, 0, 0});
}
uint8_t info::rotation(const block_t block, const enums::Face face) const
{
	return rotation_util::face_rotation_LUT.at(rotation(block))[face];
}

bool info::selectable(const block_t block) const
{
	return get_value(selectable_, block, true);
}
void info::selectable(const block_t block, const bool value)
{
	set_value(selectable_, block, value, true);
}

glm::dvec4 info::selection_color(const block_t block) const
{
	return get_value(selection_color_, block, {1, 1, 1, 1});
}
void info::selection_color(const block_t block, const glm::dvec4& value)
{
	set_value(selection_color_, block, value, {1, 1, 1, 1});
}

enums::visibility_type info::visibility_type(const block_t block) const
{
	return get_value(visibility_type_, block, enums::visibility_type::opaque);
}
void info::visibility_type(const block_t block, const enums::visibility_type value)
{
	set_value(visibility_type_, block, value, enums::visibility_type::opaque);
}

static std::size_t get_face_i
(
	const enums::Face face,
	const glm::tvec3<uint8_t>& rotation
)
{
	return static_cast<std::size_t>(rotation_util::rotate_face(face, rotation));
}

fs::path info::shader_path(const block_t block, const enums::Face face) const
{
	if(const auto i = shader_path_.find(block);
		i != shader_path_.cend())
	{
		return i->second[get_face_i(face, rotation(block))];
	}
	return "shaders/block/default";
}
void info::shader_path(const block_t block, const enums::Face face, const fs::path& value)
{
	auto i = shader_path_.find(block);
	if(i == shader_path_.cend())
	{
		i = shader_path_.emplace(block, std::array<fs::path, 6>{}).first;
	}
	// TODO: should get_face_i be used here?
	i->second[get_face_i(face, rotation(block))] = "shaders/block" / value;
}
void info::shader_path(const block_t block, const fs::path& value)
{
	auto i = shader_path_.find(block);
	if(i == shader_path_.cend())
	{
		i = shader_path_.emplace(block, std::array<fs::path, 6>{}).first;
	}
	i->second.fill("shaders/block" / value);
}

fs::path info::texture_path(const block_t block, const enums::Face face) const
{
	if(const auto i = texture_path_.find(block);
		i != texture_path_.cend())
	{
		return i->second[get_face_i(face, rotation(block))];
	}
	return "";
}
void info::texture_path(const block_t block, const enums::Face face, const fs::path& value)
{
	const auto face_i = get_face_i(face, rotation(block));

	auto i = texture_path_.find(block);
	if(i == texture_path_.cend())
	{
		i = texture_path_.emplace(block, std::array<fs::path, 6>{}).first;
	}
	const fs::path path = "textures" / value;
	i->second[face_i] = path;

	auto i2 = texture_info_.find(block);
	if(i2 == texture_info_.cend())
	{
		i2 = texture_info_.emplace(block, std::array<resource_manager::block_texture_info, 6>{}).first;
	}
	i2->second[face_i] = game::instance->resource_manager.get_block_texture(path);
}
void info::texture_path(const block_t block, const fs::path& value)
{
	auto i = texture_path_.find(block);
	if(i == texture_path_.cend())
	{
		i = texture_path_.emplace(block, std::array<fs::path, 6>{}).first;
	}
	const fs::path path = "textures" / value;
	i->second.fill(path);

	auto i2 = texture_info_.find(block);
	if(i2 == texture_info_.cend())
	{
		i2 = texture_info_.emplace(block, std::array<resource_manager::block_texture_info, 6>{}).first;
	}
	i2->second.fill(game::instance->resource_manager.get_block_texture(path));
}

resource_manager::block_texture_info info::texture_info(const block_t block, const enums::Face face) const
{
	if(const auto i = texture_info_.find(block);
		i != texture_info_.cend())
	{
		return i->second[get_face_i(face, rotation(block))];
	}
	return {};
}

}
