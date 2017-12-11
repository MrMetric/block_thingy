#include "ResourceManager.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <regex>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <concurrentqueue/concurrentqueue.hpp>

#include "Game.hpp"
#include "Settings.hpp"
#include "Util.hpp"
#include "block/Base.hpp"
#include "block/Textured.hpp"
#include "block/Enum/VisibilityType.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/Image.hpp"
#include "graphics/OpenGL/ShaderObject.hpp"
#include "graphics/OpenGL/Texture.hpp"
#include "util/filesystem.hpp"
#include "util/FileWatcher.hpp"
#include "util/logger.hpp"

using std::string;
using std::unique_ptr;
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
	std::unordered_map<string, uint16_t> index;
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

	// note: fs::path can not be a key because it can not be hashed
	std::unordered_map<string, unique_ptr<Graphics::Image>> cache_Image;
	mutable std::mutex cache_Image_mutex;

	std::unordered_map<string, unique_ptr<Graphics::OpenGL::ShaderObject>> cache_ShaderObject;
	mutable std::mutex cache_ShaderObject_mutex;

	std::unordered_map<string, unique_ptr<Graphics::OpenGL::ShaderProgram>> cache_ShaderProgram;
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
	for(string line; std::getline(ss, line);)
	{
		std::vector<string> parts = ArgumentParser().parse_args(line);
		if(parts.size() == 0) continue; // comment or empty
		if(parts.size() != 2)
		{
			// TODO
			LOG(ERROR) << "invalid line in block: " << line << '\n';
			continue;
		}
		things.emplace(parts[0], parts[1]);
	}
	return things;
}

enum class block_type
{
	invalid,
	base,
	textured
};
static block_type get_block_type(const std::unordered_map<string, string>& block)
{
	if(block.count("id") == 0)
	{
		return block_type::invalid;
	}

	bool has_texture = block.count("texture") != 0;
	if(!has_texture)
	{
		const std::size_t sum =
		  block.count("texture_right")
		+ block.count("texture_left")
		+ block.count("texture_top")
		+ block.count("texture_bottom")
		+ block.count("texture_front")
		+ block.count("texture_back");
		has_texture = (sum == 6);
		if(!has_texture && sum != 0) // no "texture" value, and not all faces specified
		{
			return block_type::invalid;
		}
	}

	// if there is a texture, the shader has a default value, so no need to check
	if(!has_texture)
	{
		bool has_shader = block.count("shader") != 0;
		if(!has_shader)
		{
			const std::size_t sum =
			  block.count("shader_right")
			+ block.count("shader_left")
			+ block.count("shader_top")
			+ block.count("shader_bottom")
			+ block.count("shader_front")
			+ block.count("shader_back");
			has_shader = (sum == 6);
			if(!has_shader && sum != 0) // no "shader" value, and not all faces specified
			{
				return block_type::invalid;
			}
		}
	}

	if(block.count("visibility_type") != 0)
	{
		if(has_texture)
		{
			return block_type::invalid;
		}
		const string& visibility_type = block.at("visibility_type");
		if(visibility_type != "opaque"
		&& visibility_type != "translucent"
		&& visibility_type != "invisible")
		{
			return block_type::invalid;
		}
	}

	if(block.count("light") != 0)
	{
		static std::regex re_light(R"(\s*\d+\s+\d+\s+\d+\s*)");
		if(!std::regex_match(block.at("light"), re_light))
		{
			return block_type::invalid;
		}
	}

	if(has_texture)
	{
		return block_type::textured;
	}
	return block_type::base;
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

		auto block = parse_block(Util::read_text(path));
		const block_type t = get_block_type(block);
		if(t == block_type::invalid)
		{
			// TODO
			LOG(WARN) << "ignoring invalid block " << path.u8string() << '\n';
			continue;
		}
		LOG(DEBUG) << "loading block: " << path.u8string() << '\n';

		Block::Enum::VisibilityType visibility_type;
		if(block.count("visibility_type") == 0)
		{
			visibility_type = Block::Enum::VisibilityType::opaque;
		}
		else
		{
			// switch on first char works because the block validity has already been confirmed
			switch(block.at("visibility_type")[0])
			{
				case 't': visibility_type = Block::Enum::VisibilityType::translucent; break;
				case 'i': visibility_type = Block::Enum::VisibilityType::invisible; break;
				default : visibility_type = Block::Enum::VisibilityType::opaque; break;
			}
		}

		std::array<fs::path, 6> shaders;
		if(block.count("shader") != 0)
		{
			shaders.fill(block.at("shader"));
		}
		else if(t == block_type::textured)
		{
			shaders.fill("texture");
		}
		if(block.count("shader_right" ) != 0) shaders[0] = block.at("shader_right" );
		if(block.count("shader_left"  ) != 0) shaders[1] = block.at("shader_left"  );
		if(block.count("shader_top"   ) != 0) shaders[2] = block.at("shader_top"   );
		if(block.count("shader_bottom") != 0) shaders[3] = block.at("shader_bottom");
		if(block.count("shader_front" ) != 0) shaders[4] = block.at("shader_front" );
		if(block.count("shader_back"  ) != 0) shaders[5] = block.at("shader_back"  );

		Block::Enum::Type id;
		if(t == block_type::textured)
		{
			std::array<fs::path, 6> textures;
			if(block.count("texture") != 0)
			{
				textures.fill(block.at("texture"));
			}
			if(block.count("texture_right" ) != 0) textures[0] = block.at("texture_right" );
			if(block.count("texture_left"  ) != 0) textures[1] = block.at("texture_left"  );
			if(block.count("texture_top"   ) != 0) textures[2] = block.at("texture_top"   );
			if(block.count("texture_bottom") != 0) textures[3] = block.at("texture_bottom");
			if(block.count("texture_front" ) != 0) textures[4] = block.at("texture_front" );
			if(block.count("texture_back"  ) != 0) textures[5] = block.at("texture_back"  );
			id = game.block_registry.add<Block::Textured>
			(
				block.at("id"),
				textures,
				shaders
			);
		}
		else if(t == block_type::base)
		{
			id = game.block_registry.add<Block::Base>
			(
				block.at("id"),
				visibility_type,
				shaders
			);
		}
		else
		{
			LOG(BUG) << "attempted to load unhandled block type: " << std::to_string(static_cast<int>(t)) << '\n';
			continue;
		}

		if(block.count("light") != 0)
		{
			// light.{r,g,b} is uint8_t, which is char, so stringstream treats the input as characters instead of digits
			unsigned int r, g, b;
			std::istringstream(block.at("light")) >> r >> g >> b;
			#define c(x) static_cast<uint8_t>(x)
			game.block_registry.get_default(id)->light({c(r), c(g), c(b)});
			#undef c
		}
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
	const auto i = t.index.find(path.string());
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
				LOG(ERROR) << "error reloading " << path.u8string() << ": bad block texture dimensions: " << res << "×" << image->get_height() << '\n';
				return;
			}
			if(res != old_res)
			{
				// TODO: allow this
				LOG(ERROR) << "error reloading " << path.u8string() << ": the dimensions changed (old: " << old_res << "×; new: " << res << "×)\n";
			}
			std::lock_guard<std::mutex> g(pImpl->block_textures_mutex);
			block_texture& t = pImpl->get_block_texture(res);
			glActiveTexture(GL_TEXTURE0 + t.unit);
			t.tex.image3D_sub(0, 0, 0, depth, res, res, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->get_data());
			glActiveTexture(GL_TEXTURE0);
			LOG(INFO) << "reloaded " << path.u8string() << " (layer " << depth << " of unit " << std::to_string(t.unit) << ")\n";
		});
		glActiveTexture(GL_TEXTURE0 + t.unit);
		t.tex.image3D_sub(0, 0, 0, depth, res, res, 1, GL_RGBA, GL_UNSIGNED_BYTE, image->get_data());
		glActiveTexture(GL_TEXTURE0);
		LOG(DEBUG) << "loaded " << path.u8string() << " as layer " << depth << " of unit " << std::to_string(t.unit) << '\n';
	});
	t.index.emplace(path.string(), depth);
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
	return pImpl->cache_Image.find(path.string()) != pImpl->cache_Image.cend();
}

Resource<Graphics::Image> ResourceManager::get_Image(const fs::path& path, const bool reload)
{
	std::lock_guard<std::mutex> g(pImpl->cache_Image_mutex);
	auto i = pImpl->cache_Image.find(path.string());
	if(i == pImpl->cache_Image.cend())
	{
		i = pImpl->cache_Image.emplace(path.string(), std::make_unique<Graphics::Image>(path)).first;
		pImpl->file_watcher.add_watch(path);
	}
	else if(reload)
	{
		unique_ptr<Graphics::Image> p;
		try
		{
			p = std::make_unique<Graphics::Image>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "failed to update image " << path.u8string() << ": " << e.what() << '\n';
			return Resource<Graphics::Image>(&i->second, path.string());
		}
		std::swap(p, i->second);
		Resource<Graphics::Image> r(&i->second, path.string());
		r.update();
		return r;
	}
	return Resource<Graphics::Image>(&i->second, path.string());
}

bool ResourceManager::has_ShaderObject(const fs::path& path) const
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderObject_mutex);
	return pImpl->cache_ShaderObject.find(path.string()) != pImpl->cache_ShaderObject.cend();
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
	if(!fs::exists(path))
	{
		const fs::path orig = path;
		path.replace_filename("default" + path.extension().string()); // no replace_stem?
		if(!fs::exists(path))
		{
			throw std::runtime_error("shader not found and no default exists: " + orig.u8string());
		}
	}
	auto i = pImpl->cache_ShaderObject.find(path.string());
	if(i == pImpl->cache_ShaderObject.cend())
	{
		// the shader object is not in the cache, so compile and add it
		i = pImpl->cache_ShaderObject.emplace(path.string(), std::make_unique<ShaderObject>(path, type)).first;
		pImpl->file_watcher.add_watch(path);
	}
	else if(reload)
	{
		unique_ptr<ShaderObject> p;
		try
		{
			p = std::make_unique<ShaderObject>(path, type);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading ShaderObject " << path.u8string() << ": " << e.what() << '\n';
			return Resource<ShaderObject>(&i->second, path.string());
		}
		std::swap(p, i->second);
		Resource<ShaderObject> r(&i->second, path.string());
		r.update();
		return r;
	}
	return Resource<ShaderObject>(&i->second, path.string());
}

bool ResourceManager::has_ShaderProgram(const fs::path& path) const
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderProgram_mutex);
	return pImpl->cache_ShaderProgram.find(path.string()) != pImpl->cache_ShaderProgram.cend();
}

Resource<ShaderProgram> ResourceManager::get_ShaderProgram(const fs::path& path, bool reload)
{
	std::lock_guard<std::mutex> g(pImpl->cache_ShaderProgram_mutex);
	auto i = pImpl->cache_ShaderProgram.find(path.string());
	if(i == pImpl->cache_ShaderProgram.cend())
	{
		i = pImpl->cache_ShaderProgram.emplace(path.string(), std::make_unique<ShaderProgram>(path)).first;

		// TODO: find a better place for this
	#ifdef _WIN32
		if(Util::string_starts_with(path.string(), "shaders\\block\\"))
	#else
		if(Util::string_starts_with(path, "shaders/block/"))
	#endif
		{
			i->second->uniform("light", 1); // the texture unit
			i->second->uniform("light_smoothing", static_cast<int>(Settings::get<int64_t>("light_smoothing")));
			i->second->uniform("min_light", static_cast<float>(Settings::get<double>("min_light")));
		}
	}
	else if(reload)
	{
		unique_ptr<ShaderProgram> p;
		try
		{
			p = std::make_unique<ShaderProgram>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading ShaderProgam " << path.u8string() << ": " << e.what() << '\n';
			return Resource<ShaderProgram>(&i->second, path.string());
		}
		std::swap(p, i->second);
		Resource<ShaderProgram> r(&i->second, path.string());
		r.update();
		return r;
	}
	return Resource<ShaderProgram>(&i->second, path.string());
}

void ResourceManager::foreach_ShaderProgram(const std::function<void(Resource<Graphics::OpenGL::ShaderProgram>)>& f)
{
	for(auto& p : pImpl->cache_ShaderProgram)
	{
		f({&p.second, p.first});
	}
}
