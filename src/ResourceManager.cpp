#include "ResourceManager.hpp"

#include <atomic>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include <concurrentqueue/concurrentqueue.hpp>
#include <easylogging++/easylogging++.hpp>

#include "Game.hpp"
#include "Util.hpp"
#include "block/Base.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/Image.hpp"
#include "graphics/OpenGL/ShaderObject.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "util/filesystem.hpp"
#include "util/FileWatcher.hpp"

using std::cerr;
using std::string;
using Graphics::OpenGL::ShaderObject;

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
		units(0)
	{
	}

	Util::FileWatcher file_watcher;
	moodycamel::ConcurrentQueue<std::function<void()>> work;
	const std::thread::id main_thread_id;

	std::unordered_map<uint32_t, block_texture> block_textures;
	uint8_t units;
	std::mutex block_textures_mutex;

	std::unordered_map<std::string, std::unique_ptr<Graphics::Image>> cache_Image;
	std::unordered_map<std::string, std::unique_ptr<Graphics::OpenGL::ShaderObject>> cache_ShaderObject;
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
		game.add_block(block["name"], block["shader"]);
	}
}

ResourceManager::block_texture_info ResourceManager::get_block_texture(fs::path path)
{
	if(path.empty())
	{
		return {0, 0};
	}

	path = "textures" / path;

	std::lock_guard<std::mutex> g(pImpl->block_textures_mutex);

	Resource<Graphics::Image> image = get_Image(path);
	const auto res = image->get_width();
	if(res != image->get_height())
	{
		throw std::runtime_error("bad block texture dimensions: " + std::to_string(res) + "×" + std::to_string(image->get_height()));
	}

	auto i = pImpl->block_textures.find(res);
	if(i == pImpl->block_textures.cend())
	{
		if(std::this_thread::get_id() == pImpl->main_thread_id)
		{
			i = pImpl->block_textures.emplace(res, block_texture(++pImpl->units, res)).first;
		}
		else
		{
			std::atomic<bool> done(false);
			pImpl->work.enqueue([this, res, &done]()
			{
				pImpl->block_textures.emplace(res, block_texture(++pImpl->units, res));
				done = true;
			});
			while(!done);
			i = pImpl->block_textures.find(res);
		}
	}

	block_texture& t = i->second;
	const auto i2 = t.index.find(path);
	if(i2 != t.index.cend())
	{
		return
		{
			t.unit,
			i2->second,
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
			block_texture& t = pImpl->block_textures.at(res);
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
	return {t.unit, depth};
}

bool ResourceManager::texture_has_transparency(const fs::path& path)
{
	return get_Image("textures" / path)->has_transparency();
}

Resource<Graphics::Image> ResourceManager::get_Image(const fs::path& path, const bool reload)
{
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

Resource<ShaderObject> ResourceManager::get_ShaderObject(fs::path path, const bool reload)
{
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
			cerr << "failed to update ShaderObject " << path.u8string() << ": " << e.what() << "\n";
			return Resource<ShaderObject>(&i->second, path);
		}
		std::swap(p, i->second);
		Resource<ShaderObject> r(&i->second, path);
		r.update();
		return r;
	}
	return Resource<ShaderObject>(&i->second, path);
}
