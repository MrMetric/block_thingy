#pragma once

#include <memory>
#include <experimental/propagate_const>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fwd/Game.hpp"
#include "fwd/graphics/OpenGL/ShaderObject.hpp"
#include "util/filesystem.hpp"

template<typename T>
class Resource
{
	public:
		using update_func_t = std::function<void()>;

		Resource(std::unique_ptr<T>* p, const std::string& id)
		:
			id(id)
		{
			this->p = p;
			update_funcs.emplace(id, std::vector<update_func_t>(1));
		}

		T& operator*()
		{
			return **p;
		}

		T* operator->()
		{
			return p->get();
		}

		void on_update(update_func_t f)
		{
			update_funcs[id].emplace_back(f);
		}

		void update()
		{
			for(const update_func_t& f : update_funcs[id])
			{
				if(f)
				{
					f();
				}
			}
		}

		const std::string id;

	private:
		static std::unordered_map<std::string, std::vector<update_func_t>> update_funcs;
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

	Resource<Graphics::OpenGL::ShaderObject> get_ShaderObject(fs::path, bool reload = false);

private:
	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
