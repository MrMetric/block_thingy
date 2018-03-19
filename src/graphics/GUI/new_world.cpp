#include "new_world.hpp"

#include <algorithm>
#include <chrono>
#include <limits>
#include <sstream>
#include <tuple>

#include "game.hpp"
#include "language.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "graphics/GUI/Widget/Text.hpp"
#include "graphics/GUI/Widget/text_input.hpp"
#include "util/crc32.hpp"
#include "util/filesystem.hpp"
#include "util/logger.hpp"
#include "util/misc.hpp"

using std::string;

namespace block_thingy::graphics::gui {

static string name_to_dir(string);

enum class dir_status
{
	good,
	warn,
	error,
};
static std::tuple<dir_status, string> get_dir_status(const string&);

static double get_seed(const string&);

new_world::new_world(game& g)
:
	Base(g, "guis/new_world.btgui")
{
	auto* name = root.get_widget_by_id_t<widget::text_input>("name");
	if(name == nullptr)
	{
		LOG(ERROR) << "name text input not found in GUI 'new_world'\n";
	}

	auto* dir = root.get_widget_by_id_t<widget::text_input>("dir");
	if(dir == nullptr)
	{
		LOG(ERROR) << "dir text input not found in GUI 'new_world'\n";
	}

	auto* seed = root.get_widget_by_id_t<widget::text_input>("seed");
	if(seed == nullptr)
	{
		LOG(ERROR) << "seed text input not found in GUI 'new_world'\n";
	}

	auto* seed_display = root.get_widget_by_id_t<widget::Text>("seed_display");
	if(seed_display == nullptr)
	{
		LOG(ERROR) << "seed_display text not found in GUI 'new_world'\n";
	}

	auto* error = root.get_widget_by_id_t<widget::Text>("error");
	if(error == nullptr)
	{
		LOG(ERROR) << "error text not found in GUI 'new_world'\n";
	}

	auto* confirm = root.get_widget_by_id_t<widget::Button>("confirm");
	if(confirm == nullptr)
	{
		LOG(ERROR) << "confirm button not found in GUI 'new_world'\n";
	}

	// not all of these are strictly necessary, but it is easier to assume that all exist
	if(name == nullptr
	|| dir == nullptr
	|| seed == nullptr
	|| seed_display == nullptr
	|| error == nullptr
	|| confirm == nullptr)
	{
		return;
	}

	name->on_change([dir](widget::text_input&, const string& old_value, const string& new_value)
	{
		const fs::path old_dir = name_to_dir(old_value);
		const fs::path new_dir = name_to_dir(new_value);
		if(dir->get_text() == old_dir.u8string())
		{
			dir->set_text(new_dir.u8string());
		}
	});
	dir->on_change([this, error, confirm](widget::text_input& dir, const string& /*old_value*/, const string& new_value)
	{
		const string dir_name = name_to_dir(new_value);
		if(dir.get_text() != dir_name)
		{
			dir.set_text(dir_name, false);
		}

		const auto [status, message] = get_dir_status(dir_name);
		const bool valid = (status != dir_status::error);
		dir.valid(valid);
		confirm->enabled(valid);
		error->set_text(message);
		refresh_layout();
	});
	seed->on_change([seed_display](widget::text_input& /*seed*/, const string& /*old_value*/, const string& new_value)
	{
		const double seed = get_seed(new_value);
		if(seed == 0)
		{
			seed_display->set_text(" = random");
		}
		else
		{
			std::ostringstream ss;
			ss.precision(std::numeric_limits<double>::max_digits10);
			ss << seed;
			seed_display->set_text(" = " + ss.str());
		}
	});
	confirm->on_click([this, &g, name, dir, seed, error](widget::Button&, const glm::dvec2& /*position*/)
	{
		const string dir_name = dir->get_text();
		try
		{
			fs::create_directory("worlds/" + dir_name);
		}
		catch(const fs::filesystem_error& e)
		{
			error->set_text(language::get("gui.new_world.dir_create_error") + '\n' + e.what());
			refresh_layout();
			return;
		}
		double seed_value = get_seed(seed->get_text());
		if(seed_value == 0)
		{
			// the time is random enough, right?
			const auto now = std::chrono::system_clock::now();
			const auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
			const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms.time_since_epoch());
			seed_value = duration.count();
		}
		g.new_world(dir_name, name->get_text(), seed_value);
	});
}

string new_world::type() const
{
	return "new_world";
}

string name_to_dir(string name)
{
	std::replace_if(name.begin(), name.end(), [](const char& c)
	{
		return
			c == 0
		 || c == '/'
	#ifdef _WIN32
		 || c < 32
		 || c == '\\'
		 || c == ':'
		 || c == '*'
		 || c == '?'
		 || c == '"'
		 || c == '<'
		 || c == '>'
		 || c == '|'
	#endif
		;
	}, '_');

	return name;
}

std::tuple<dir_status, string> get_dir_status(const string& dir_name)
{
	if(dir_name.empty())
	{
		return
		{
			dir_status::error,
			language::get("gui.new_world.dir_name_required"),
		};
	}

	try
	{
		const fs::path dir_path("worlds/" + dir_name);
		if(fs::is_regular_file(dir_path / "world"))
		{
			return
			{
				dir_status::error,
				language::get("gui.new_world.world_exists"),
			};
		}
		if(fs::is_directory(dir_path))
		{
			return
			{
				dir_status::warn,
				language::get("gui.new_world.dir_exists"),
			};
		}
		if(fs::exists(dir_path))
		{
			return
			{
				dir_status::error,
				language::get("gui.new_world.file_conflict"),
			};
		}
	}
	catch(const fs::filesystem_error& e)
	{
		return
		{
			dir_status::error,
			e.what(),
		};
	}

	return
	{
		dir_status::good,
		{},
	};
}

double get_seed(const string& s)
{
	if(s.empty())
	{
		return 0;
	}
	if(const std::optional<double> d = util::stod(s); d.has_value())
	{
		return *d;
	}
	return util::crc32(s);
}

}
