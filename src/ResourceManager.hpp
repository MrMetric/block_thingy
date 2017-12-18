#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fwd/Game.hpp"
#include "fwd/graphics/Image.hpp"
#include "fwd/graphics/OpenGL/ShaderObject.hpp"
#include "fwd/graphics/OpenGL/ShaderProgram.hpp"
#include "shim/propagate_const.hpp"
#include "util/filesystem.hpp"

namespace block_thingy {

template<typename T>
class resource
{
public:
	using update_func_t = std::function<void()>;

	resource(std::unique_ptr<T>* p, const std::string& id)
	:
		id(id),
		p(p)
	{
		update_funcs.emplace(id, std::vector<update_func_t>(1));
	}

	T& operator*() const
	{
		return **p;
	}

	T* operator->() const
	{
		return p->get();
	}

	void on_update(update_func_t f) const
	{
		update_funcs[id].emplace_back(f);
	}

	void update() const
	{
		for(const update_func_t& f : update_funcs[id])
		{
			if(f)
			{
				f();
			}
		}
	}

	std::string get_id() const
	{
		return id;
	}

private:
	static std::unordered_map<std::string, std::vector<update_func_t>> update_funcs;
	std::string id;
	std::unique_ptr<T>* p;
};
template<typename T>
std::unordered_map<std::string, std::vector<typename resource<T>::update_func_t>> resource<T>::update_funcs;

class resource_manager
{
public:
	resource_manager();
	~resource_manager();

	void check_updates();

	void load_blocks(game&);

	struct block_texture_info
	{
		uint8_t unit;
		uint16_t index;
	};
	block_texture_info get_block_texture(fs::path);
	bool texture_has_transparency(const fs::path&);

	bool has_image(const fs::path&) const;
	resource<graphics::image> get_image(const fs::path&, bool reload = false);

	bool has_shader_object(const fs::path&) const;
	resource<graphics::opengl::shader_object> get_shader_object(fs::path, bool reload = false);

	bool has_shader_program(const fs::path&) const;
	// TODO: shaders can be constructed with a file list
	resource<graphics::opengl::shader_program> get_shader_program(const fs::path&, bool reload = false);
	void foreach_shader_program(const std::function<void(resource<graphics::opengl::shader_program>)>&);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
