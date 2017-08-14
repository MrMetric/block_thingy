#include "ResourceManager.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <concurrentqueue/concurrentqueue.hpp>
#include <easylogging++/easylogging++.hpp>

#include "Game.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "block/SimpleShader.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/Image.hpp"
#include "graphics/OpenGL/ShaderObject.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "util/filesystem.hpp"
#include "util/FileWatcher.hpp"

using std::cerr;
using std::string;
using Graphics::OpenGL::ShaderObject;
using Graphics::OpenGL::ShaderProgram;

struct block_texture
{
	block_texture
	(
		const uint8_t unit,
		const uint32_t size
	)
	:
		unit(unit),
		tex(GL_TEXTURE_2D_ARRAY),
		count(0)
	{
		tex.parameter(Graphics::OpenGL::Texture::Parameter::wrap_s, GL_REPEAT);
		tex.parameter(Graphics::OpenGL::Texture::Parameter::wrap_t, GL_REPEAT);
		tex.parameter(Graphics::OpenGL::Texture::Parameter::min_filter, GL_LINEAR);
		tex.parameter(Graphics::OpenGL::Texture::Parameter::mag_filter, GL_NEAREST);
		glActiveTexture(GL_TEXTURE0 + unit);
		tex.image3D(0, GL_RGBA8, size, size, 256, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glActiveTexture(GL_TEXTURE0);
	}

	block_texture(block_texture&& that)
	:
		unit(that.unit),
		tex(std::move(that.tex)),
		count(that.count),
		index(std::move(that.index))
	{
	}
	block_texture(const block_texture&) = delete;
	block_texture& operator=(block_texture&&) = delete;
	block_texture& operator=(const block_texture&) = delete;

	const uint8_t unit;
	Graphics::OpenGL::Texture tex;
	uint16_t count;
	std::unordered_map<std::string, uint16_t> index;
};

struct ResourceManager::impl
{
	impl()
	:
		main_thread_id(std::this_thread::get_id()),
		units(1)
	{
	}

	block_texture& get_block_texture(uint32_t res)
	{
		const auto i = block_textures.find(res);
		if(i != block_textures.cend())
		{
			return i->second;
		}

		if(std::this_thread::get_id() == main_thread_id)
		{
			return block_textures.emplace(res, block_texture(++units, res)).first->second;
		}

		std::atomic<bool> done(false);
		work.enqueue([this, res, &done]()
		{
			block_textures.emplace(res, block_texture(++units, res));
			done = true;
		});
		while(!done)
		{
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(2ms);
		}
		return block_textures.at(res);
	}

	Util::FileWatcher file_watcher;
	moodycamel::ConcurrentQueue<std::function<void()>> work;
	const std::thread::id main_thread_id;

	std::unordered_map<uint32_t, block_texture> block_textures;
	uint8_t units;
	mutable std::mutex block_textures_mutex;

	std::unordered_map<std::string, std::unique_ptr<Graphics::Image>> cache_Image;
	mutable std::mutex cache_Image_mutex;

	std::unordered_map<std::string, std::unique_ptr<Graphics::OpenGL::ShaderObject>> cache_ShaderObject;
	mutable std::mutex cache_ShaderObject_mutex;

	std::unordered_map<std::string, std::unique_ptr<Graphics::OpenGL::ShaderProgram>> cache_ShaderProgram;
	mutable std::mutex cache_ShaderProgram_mutex;
};

ResourceManager::ResourceManager()
:
	pImpl(std::make_unique<impl>())
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::check_updates()
{
	std::vector<fs::path> paths = pImpl->file_watcher.get_updates();
	for(const fs::path& path : paths)
	{
		if(path.extension() == ".fs"
		|| path.extension() == ".vs")
		{
			get_ShaderObject(path, true);
		}
		else if(path.extension() == ".png")
		{
			get_Image(path, true);
		}
	}

	std::function<void()> f;
	while(pImpl->work.try_dequeue(f))
	{
		f();
	}
}

static std::unordered_map<string, string> parse_block(const string& s)
{
	std::unordered_map<string, string> things;

	std::istringstream ss(s);
	for(std::string line; std::getline(ss, line);)
	{
		std::vector<string> parts = ArgumentParser().parse_args(line);
		if(parts.size() == 0) continue; // comment or empty
		if(parts.size() != 2)
		{
			// TODO
			cerr << "invalid line: " << line << "\n";
			continue;
		}
		things.emplace(parts[0], parts[1]);
	}
	return things;
}

static bool is_block_valid(const std::unordered_map<string, string>& block)
{
	if(block.count("name") != 1) return false;
	if(block.count("shader") != 1) return false;
	return true;
}

void ResourceManager::load_blocks(Game& game)
{
	for(const fs::directory_entry& entry : fs::recursive_directory_iterator("blocks"))
	{
		const fs::path path = entry.path();
		if(path.extension() != ".btblock")
		{
			continue;
		}

		auto block = parse_block(Util::read_file(path));
		if(!is_block_valid(block))
		{
			// TODO
			LOG(WARNING) << "ignoring invalid block " << path.u8string();
			continue;
		}
		game.block_registry.add<Block::SimpleShader>(block["name"], block["shader"]);
	}
}

ResourceManager::block_texture_info ResourceManager::get_block_texture(fs::path path)
{
	if(path.empty())
	{
		return {0, 0};
	}

	path = "textures" / path;

	Resource<Graphics::Image> image = get_Image(path);
	const auto res = image->get_width();
	if(res != image->get_height())
	{
		throw std::runtime_error("bad block texture dimensions: " + std::to_string(res) + "×" + std::to_string(image->get_height()));
	}

	std::lock_guard<std::mutex> g(pImpl->block_textures_mutex);

	block_texture& t = pImpl->get_block_texture(res);
	const auto i = t.index.find(path);
	if(i != t.index.cend())
	{
		return
		{
			t.unit,
			i->second,
		};
	}

	const auto depth = t.count++;
	pImpl->work.enqueue(
	[
		this,
		path,
		image,
		res,
		&t,
		depth
	]()
	{
		image.on_update(
		[
			this,
			path,
			image,
			old_res=res,
			depth
		]()
		{
			const auto res = image->get_width();
			if(res != image->get_height())
			{
				LOG(ERROR) << "error reloading " << path.u8string() << ": bad block texture dimensions: " << res << "×" << image->get_height();
				return;
			}
			if(res != old_res)
			{
				// TODO: allow this
				LOG(ERROR) << "error reloading " << path.u8string() << ": the dimensions changed (old: " << old_res << "×; new: " << res << "×)";
			}
			std::lock_guard<std::mutex> g(pImpl->block_textures_mutex);
			block_texture& t = pImpl->get_block_texture(res);
			glActiveTexture(GL_TEXTURE0 + t.unit);
			t.tex.image3D_sub(0, 0, 0, depth, res, res, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->get_data());
			glActiveTexture(GL_TEXTURE0);
			LOG(INFO) << "reloaded " << path.u8string() << " (layer " << depth << " of unit " << std::to_string(t.unit) << ')';
		});
		glActiveTexture(GL_TEXTURE0 + t.unit);
		t.tex.image3D_sub(0, 0, 0, depth, res, res, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->get_data());
		glActiveTexture(GL_TEXTURE0);
		LOG(INFO) << "loaded " << path.u8string() << " as layer " << depth << " of unit " << std::to_string(t.unit);
	});
	t.index.emplace(path, depth);
	return
	{
		t.unit,
		depth,
	};
}

bool ResourceManager::texture_has_transparency(const fs::path& path)
{
	return get_Image("textures" / path)->has_transparency();
}

bool ResourceManager::has_Image(const fs::path& path) const
{
	std::lock_guard<std::mutex> g(pImpl->cache_Image_mutex);
	return pImpl->cache_Image.find(path) != pImpl->cache_Image.cend();
}

Resource<Graphics::Image> ResourceManager::get_Image(const fs::path& path, const bool reload)
{
	std::lock_guard<std::mutex> g(pImpl->cache_Image_mutex);
	auto i = pImpl->cache_Image.find(path);
	if(i == pImpl->cache_Image.cend())
	{
		i = pImpl->cache_Image.emplace(path, std::make_unique<Graphics::Image>(path)).first;
		pImpl->file_watcher.add_watch(path);
	}
	else if(reload)
	{
		std::unique_ptr<Graphics::Image> p;
		try
		{
			p = std::make_unique<Graphics::Image>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "failed to update image " << path.u8string() << ": " << e.what();
			return Resource<Graphics::Image>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<Graphics::Image> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<Graphics::Image>(&i->second, path);
}

bool ResourceManager::has_ShaderObject(const fs::path& path) const
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderObject_mutex);
	return pImpl->cache_ShaderObject.find(path) != pImpl->cache_ShaderObject.cend();
}

Resource<ShaderObject> ResourceManager::get_ShaderObject(fs::path path, const bool reload)
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderObject_mutex);
	GLenum type;
	if(path.extension() == ".fs")
	{
		type = GL_FRAGMENT_SHADER;
	}
	else if(path.extension() == ".vs")
	{
		type = GL_VERTEX_SHADER;
	}
	else
	{
		throw std::invalid_argument("bad shader path: " + path.u8string());
	}
	if(!Util::file_is_openable(path))
	{
		const fs::path orig = path;
		path.replace_filename("default" + path.extension().string()); // no replace_stem?
		if(!Util::file_is_openable(path))
		{
			throw std::runtime_error("shader not found and no default exists: " + orig.u8string());
		}
	}
	auto i = pImpl->cache_ShaderObject.find(path);
	if(i == pImpl->cache_ShaderObject.cend())
	{
		// the shader object is not in the cache, so compile and add it
		i = pImpl->cache_ShaderObject.emplace(path, std::make_unique<ShaderObject>(path, type)).first;
		pImpl->file_watcher.add_watch(path);
	}
	else if(reload)
	{
		std::unique_ptr<ShaderObject> p;
		try
		{
			p = std::make_unique<ShaderObject>(path, type);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading ShaderObject " << path.u8string() << ": " << e.what();
			return Resource<ShaderObject>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<ShaderObject> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<ShaderObject>(&i->second, path);
}

bool ResourceManager::has_ShaderProgram(const fs::path& path) const
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderProgram_mutex);
	return pImpl->cache_ShaderProgram.find(path) != pImpl->cache_ShaderProgram.cend();
}

Resource<ShaderProgram> ResourceManager::get_ShaderProgram(const fs::path& path, bool reload)
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderProgram_mutex);
	auto i = pImpl->cache_ShaderProgram.find(path);
	if(i == pImpl->cache_ShaderProgram.cend())
	{
		i = pImpl->cache_ShaderProgram.emplace(path, std::make_unique<ShaderProgram>(path)).first;

		// TODO: find a better place for this
		if(Util::string_starts_with(path, "shaders/block/"))
		{
			i->second->uniform("light", 1); // the texture unit
			i->second->uniform("light_smoothing", static_cast<int>(Settings::get<int64_t>("light_smoothing")));
			i->second->uniform("min_light", static_cast<float>(Settings::get<double>("min_light")));
		}
	}
	else if(reload)
	{
		std::unique_ptr<ShaderProgram> p;
		try
		{
			p = std::make_unique<ShaderProgram>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading ShaderProgam " << path.u8string() << ": " << e.what();
			return Resource<ShaderProgram>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<ShaderProgram> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<ShaderProgram>(&i->second, path);
}

void ResourceManager::foreach_ShaderProgram(const std::function<void(Resource<Graphics::OpenGL::ShaderProgram>)>& f)
{
	for(auto& p : pImpl->cache_ShaderProgram)
	{
		f({&p.second, p.first});
	}
}
