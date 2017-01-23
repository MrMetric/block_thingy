#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fwd/Game.hpp"
#include "fwd/graphics/OpenGL/ShaderObject.hpp"

namespace ResourceManager {

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
			update_funcs.emplace(id, 1);
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

void init();
void check_updates();

void load_blocks(Game&);

Resource<Graphics::OpenGL::ShaderObject> get_ShaderObject(std::string path, bool reload = false);

}
