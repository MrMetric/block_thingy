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
class Resource
{
public:
	using update_func_t = std::function<void()>;

	Resource(std::unique_ptr<T>* p, const std::string& id)
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
std::unordered_map<std::string, std::vector<typename Resource<T>::update_func_t>> Resource<T>::update_funcs;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void check_updates();

	void load_blocks(Game&);

	struct block_texture_info
	{
		uint8_t unit;
		uint16_t index;
	};
	block_texture_info get_block_texture(fs::path);
	bool texture_has_transparency(const fs::path&);

	bool has_Image(const fs::path&) const;
	Resource<graphics::Image> get_Image(const fs::path&, bool reload = false);

	bool has_ShaderObject(const fs::path&) const;
	Resource<graphics::opengl::ShaderObject> get_ShaderObject(fs::path, bool reload = false);

	bool has_ShaderProgram(const fs::path&) const;
	// TODO: shaders can be constructed with a file list
	Resource<graphics::opengl::ShaderProgram> get_ShaderProgram(const fs::path&, bool reload = false);
	void foreach_ShaderProgram(const std::function<void(Resource<graphics::opengl::ShaderProgram>)>&);

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
