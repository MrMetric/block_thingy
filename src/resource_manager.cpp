#include "resource_manager.hpp"

#include <atomic>
#include <chrono>
#include <regex>
#include <sstream>
#include <thread>
#include <unordered_map>

#include "game.hpp"
#include "Gfx.hpp"
#include "settings.hpp"
#include "block/base.hpp"
#include "block/textured.hpp"
#include "block/enums/visibility_type.hpp"
#include "console/ArgumentParser.hpp"
#include "graphics/image.hpp"
#include "graphics/opengl/shader_object.hpp"
#include "graphics/opengl/shader_program.hpp"
#include "graphics/opengl/texture.hpp"
#include "util/filesystem.hpp"
#include "util/FileWatcher.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;
using std::unique_ptr;

namespace block_thingy {

using graphics::opengl::shader_object;
using graphics::opengl::shader_program;

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
		tex.parameter(graphics::opengl::texture::Parameter::wrap_s, GL_REPEAT);
		tex.parameter(graphics::opengl::texture::Parameter::wrap_t, GL_REPEAT);
		tex.parameter(graphics::opengl::texture::Parameter::min_filter, GL_LINEAR);
		tex.parameter(graphics::opengl::texture::Parameter::mag_filter, GL_NEAREST);
		glActiveTexture(GL_TEXTURE0 + unit);
		tex.image3D(0, GL_SRGB8_ALPHA8, size, size, 256, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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
	graphics::opengl::texture tex;
	uint16_t count;
	std::unordered_map<string, uint16_t> index;
};

struct resource_manager::impl
{
	impl()
	:
		main_thread_id(std::this_thread::get_id()),
		units(1)
	{
	}

	impl(impl&&) = delete;
	impl(const impl&) = delete;
	impl& operator=(impl&&) = delete;
	impl& operator=(const impl&) = delete;

	block_texture& get_block_texture(uint32_t res)
	{
		if(const auto i = block_textures.find(res);
			i != block_textures.cend())
		{
			return i->second;
		}
		return block_textures.emplace(res, block_texture(++units, res)).first->second;
	}

	util::file_watcher file_watcher;
	const std::thread::id main_thread_id;

	std::unordered_map<uint32_t, block_texture> block_textures;
	uint8_t units;

	// note: fs::path can not be a key because it can not be hashed
	std::unordered_map<string, unique_ptr<graphics::image>> cache_image;
	std::unordered_map<string, unique_ptr<shader_object>> cache_shader_object;
	std::unordered_map<string, unique_ptr<shader_program>> cache_shader_program;
};

resource_manager::resource_manager()
:
	pImpl(std::make_unique<impl>())
{
}

resource_manager::~resource_manager()
{
}

void resource_manager::check_updates()
{
	std::vector<fs::path> paths = pImpl->file_watcher.get_updates();
	for(const fs::path& path : paths)
	{
		if(path.extension() == ".fs"
		|| path.extension() == ".vs")
		{
			get_shader_object(path, true);
		}
		else if(path.extension() == ".png")
		{
			get_image(path, true);
		}
	}
}

static std::unordered_map<string, string> parse_block(const string& s)
{
	std::unordered_map<string, string> things;

	std::istringstream ss(s);
	for(string line; std::getline(ss, line);)
	{
		std::vector<string> parts = ArgumentParser().parse_args(line);
		if(parts.empty()) continue; // comment or empty
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

void resource_manager::load_blocks(game& game)
{
	for(const fs::directory_entry& entry : fs::recursive_directory_iterator("blocks"))
	{
		const fs::path path = entry.path();
		if(path.extension() != ".btblock")
		{
			continue;
		}

		auto block = parse_block(util::read_text(path));
		const block_type t = get_block_type(block);
		if(t == block_type::invalid)
		{
			// TODO
			LOG(WARN) << "ignoring invalid block " << path.u8string() << '\n';
			continue;
		}
		LOG(DEBUG) << "loading block: " << path.u8string() << '\n';

		block::enums::visibility_type visibility_type;
		if(block.count("visibility_type") == 0)
		{
			visibility_type = block::enums::visibility_type::opaque;
		}
		else
		{
			// switch on first char works because the block validity has already been confirmed
			switch(block.at("visibility_type")[0])
			{
				case 't': visibility_type = block::enums::visibility_type::translucent; break;
				case 'i': visibility_type = block::enums::visibility_type::invisible; break;
				default : visibility_type = block::enums::visibility_type::opaque; break;
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

		block::enums::type id;
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
			id = game.block_registry.add<block::textured>
			(
				block.at("id"),
				textures,
				shaders
			);
		}
		else if(t == block_type::base)
		{
			id = game.block_registry.add<block::base>
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

resource_manager::block_texture_info resource_manager::get_block_texture(fs::path path)
{
	if(path.empty())
	{
		return {0, 0};
	}

	assert(std::this_thread::get_id() == pImpl->main_thread_id);

	path = "textures" / path;

	resource<graphics::image> image = get_image(path);
	const auto res = image->get_width();
	if(res != image->get_height())
	{
		throw std::runtime_error("bad block texture dimensions: " + std::to_string(res) + "×" + std::to_string(image->get_height()));
	}

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

	t.index.emplace(path.string(), depth);
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

	return
	{
		t.unit,
		depth,
	};
}

bool resource_manager::texture_has_transparency(const fs::path& path)
{
	return get_image("textures" / path)->has_transparency();
}

bool resource_manager::has_image(const fs::path& path) const
{
	return pImpl->cache_image.find(path.string()) != pImpl->cache_image.cend();
}

static const uint8_t MISSING_IMAGE[]
{
	0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF,
	0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
};
resource<graphics::image> resource_manager::get_image(const fs::path& path, const bool reload)
{
	auto i = pImpl->cache_image.find(path.string());
	if(i == pImpl->cache_image.cend())
	{
		unique_ptr<graphics::image> p;
		try
		{
			p = std::make_unique<graphics::image>(path);
		#ifdef BT_WATCH_IMAGES
			pImpl->file_watcher.add_watch(path);
		#endif
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "failed to load image " << path.u8string() << ": " << e.what() << '\n';
			p = std::make_unique<graphics::image>(2, 2, MISSING_IMAGE);
		#ifdef BT_WATCH_IMAGES
			// TODO: watch directory
		#endif
		}
		i = pImpl->cache_image.emplace(path.string(), std::move(p)).first;
	}
	else if(reload)
	{
		unique_ptr<graphics::image> p;
		try
		{
			p = std::make_unique<graphics::image>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "failed to update image " << path.u8string() << ": " << e.what() << '\n';
			return resource<graphics::image>(&i->second, path.string());
		}
		std::swap(p, i->second);
		resource<graphics::image> r(&i->second, path.string());
		r.update();
		return r;
	}
	return resource<graphics::image>(&i->second, path.string());
}

bool resource_manager::has_shader_object(const fs::path& path) const
{
	return pImpl->cache_shader_object.find(path.string()) != pImpl->cache_shader_object.cend();
}

resource<shader_object> resource_manager::get_shader_object(fs::path path, const bool reload)
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
	if(!fs::exists(path))
	{
		const fs::path orig = path;
		path.replace_filename("default" + path.extension().string()); // no replace_stem?
		if(!fs::exists(path))
		{
			throw std::runtime_error("shader not found and no default exists: " + orig.u8string());
		}
	}
	auto i = pImpl->cache_shader_object.find(path.string());
	if(i == pImpl->cache_shader_object.cend())
	{
		// the shader object is not in the cache, so compile and add it
		i = pImpl->cache_shader_object.emplace(path.string(), std::make_unique<shader_object>(path, type)).first;
	#ifdef BT_WATCH_SHADERS
		pImpl->file_watcher.add_watch(path);
	#endif
	}
	else if(reload)
	{
	#ifdef BT_RELOADABLE_SHADERS
		unique_ptr<shader_object> p;
		try
		{
			p = std::make_unique<shader_object>(path, type);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading shader object " << path.u8string() << ": " << e.what() << '\n';
			return resource<shader_object>(&i->second, path.string());
		}
		std::swap(p, i->second);
		resource<shader_object> r(&i->second, path.string());
		r.update();
		return r;
	#else
		LOG(WARN) << "Tried to reload a shader object, but this feature is disabled."
					" To enable, recompile the engine with -DBT_RELOADABLE_SHADERS."
					" Path: " << path.u8string() << '\n';
	#endif
	}
	return resource<shader_object>(&i->second, path.string());
}

bool resource_manager::has_shader_program(const fs::path& path) const
{
	return pImpl->cache_shader_program.find(path.string()) != pImpl->cache_shader_program.cend();
}

resource<shader_program> resource_manager::get_shader_program(const fs::path& path, bool reload)
{
	auto i = pImpl->cache_shader_program.find(path.string());
	if(i == pImpl->cache_shader_program.cend())
	{
		i = pImpl->cache_shader_program.emplace(path.string(), std::make_unique<shader_program>(path)).first;

		// TODO: find a better place for this
	#ifdef _WIN32
		if(util::string_starts_with(path.string(), "shaders\\block\\"))
	#else
		if(util::string_starts_with(path, "shaders/block/"))
	#endif
		{
			i->second->uniform("light", 1); // the texture unit
			i->second->uniform("light_smoothing", static_cast<int>(settings::get<int64_t>("light_smoothing")));
			i->second->uniform("min_light", static_cast<float>(settings::get<double>("min_light")));
		}
		else
	#ifdef _WIN32
		if(util::string_starts_with(path.string(), "shaders\\screen\\"))
	#else
		if(util::string_starts_with(path, "shaders/screen/"))
	#endif
		{
			// the default value is 0, so setting it explicitly is not needed
			//i->second->uniform("tex", 0);
			i->second->uniform("tex_size", static_cast<glm::vec2>(Gfx::instance->window_size));
		}
	}
	else if(reload)
	{
	#ifdef BT_RELOADABLE_SHADERS
		unique_ptr<shader_program> p;
		try
		{
			p = std::make_unique<shader_program>(path);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error reloading shader program " << path.u8string() << ": " << e.what() << '\n';
			return resource<shader_program>(&i->second, path.string());
		}
		std::swap(p, i->second);
		resource<shader_program> r(&i->second, path.string());
		r.update();
		return r;
	#else
		LOG(WARN) << "Tried to reload a shader program, but this feature is disabled."
					" To enable, recompile the engine with -DBT_RELOADABLE_SHADERS."
					" Path: " << path.u8string() << '\n';
	#endif
	}
	return resource<shader_program>(&i->second, path.string());
}

void resource_manager::foreach_shader_program(const std::function<void(resource<shader_program>)>& f)
{
	for(auto& p : pImpl->cache_shader_program)
	{
		f({&p.second, p.first});
	}
}

}
