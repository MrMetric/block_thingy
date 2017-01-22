#pragma once

#include <memory>
#include <string>
#include <utility>

#include "fwd/Game.hpp"
#include "fwd/graphics/OpenGL/ShaderObject.hpp"

namespace ResourceManager {

template<typename T>
class Resource
{
	public:
		Resource(std::unique_ptr<T>* p)
		{
			set(p);
		}

		void set(std::unique_ptr<T>* p)
		{
			this->p = p;
		}

		T& operator*()
		{
			return **p;
		}

		T* operator->()
		{
			return p->get();
		}
	private:
		std::unique_ptr<T>* p;
};

void load_blocks(Game&);

Resource<Graphics::OpenGL::ShaderObject> get_ShaderObject(std::string path);

}
