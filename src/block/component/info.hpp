#pragma once
#include "base.hpp"

#include <array>
#include <map>
#include <stdint.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "resource_manager.hpp"
#include "block/block.hpp"
#include "block/enums/Face.hpp"
#include "block/enums/visibility_type.hpp"
#include "graphics/color.hpp"
#include "util/filesystem.hpp"

namespace block_thingy::block::component {

class info : public base
{
public:
	std::string get_id() const override;

	void copy(block_t in, block_t out) override;
	void copy(const block_t* in, const block_t* out, std::size_t N) override;

	void save(msgpack::packer<std::ofstream>&) const override;
	void load(const msgpack::object&) override;

	/*
	 * default: true
	 */
	bool solid(block_t) const;
	void solid(block_t, bool);

	/*
	 * default: 0
	 */
	double bounciness(block_t) const;
	void bounciness(block_t, double);

	/*
	 * default: {0, 0, 0}
	 */
	graphics::color light(block_t) const;
	void light(block_t, const graphics::color&);

	/*
	 * default: {graphics::color::max}
	 */
	graphics::color light_filter(block_t) const;
	void light_filter(block_t, const graphics::color&);

	/*
	 * default: {0, 0, 0}
	 */
	glm::tvec3<uint8_t> rotation(block_t) const;
	void rotation(block_t, const glm::tvec3<uint8_t>&);
	uint8_t rotation(block_t, enums::Face) const;
	//void rotate_around(block_t, enums::Face, int8_t direction);

	/*
	 * default: true
	 */
	bool selectable(block_t) const;
	void selectable(block_t, bool);

	glm::dvec4 selection_color(block_t) const;
	void selection_color(block_t, const glm::dvec4&);

	enums::visibility_type visibility_type(block_t) const;
	void visibility_type(block_t, enums::visibility_type);
	bool is_opaque(const block_t block) const
	{
		return visibility_type(block) == enums::visibility_type::opaque;
	}
	bool is_translucent(const block_t block) const
	{
		return visibility_type(block) == enums::visibility_type::translucent;
	}
	bool is_invisible(const block_t block) const
	{
		return visibility_type(block) == enums::visibility_type::invisible;
	}

	fs::path shader_path(block_t, enums::Face) const;
	void shader_path(block_t, enums::Face, const fs::path&);

	/*
	 * sets for all faces
	 */
	void shader_path(block_t, const fs::path&);

	fs::path texture_path(block_t, enums::Face) const;
	void texture_path(block_t, enums::Face, const fs::path&);
	void texture_path(block_t, const fs::path&);

	resource_manager::block_texture_info texture_info(block_t, enums::Face) const;

private:
	std::map<block_t, bool> solid_;
	std::map<block_t, double> bounciness_;
	std::map<block_t, graphics::color> light_;
	std::map<block_t, graphics::color> light_filter_;
	std::map<block_t, glm::tvec3<uint8_t>> rotation_;
	std::map<block_t, bool> selectable_;
	std::map<block_t, glm::dvec4> selection_color_;
	std::map<block_t, enums::visibility_type> visibility_type_;
	std::map<block_t, std::array<fs::path, 6>> shader_path_;
	std::map<block_t, std::array<fs::path, 6>> texture_path_;
	std::map<block_t, std::array<resource_manager::block_texture_info, 6>> texture_info_;
};

}
