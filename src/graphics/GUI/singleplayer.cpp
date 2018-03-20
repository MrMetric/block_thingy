#include "singleplayer.hpp"

#include <algorithm>
#include <utility>

#include "game.hpp"
#include "graphics/GUI/Widget/Button.hpp"
#include "storage/world_file.hpp"
#include "util/filesystem.hpp"
#include "util/gui_parser.hpp"
#include "util/logger.hpp"

using std::string;

namespace block_thingy::graphics::gui {

singleplayer::singleplayer(game& g)
:
	Base(g, "guis/singleplayer.btgui")
{
	auto* world_list_ = root.get_widget_by_id_t<widget::Container>("world_list");
	if(world_list_ == nullptr)
	{
		LOG(ERROR) << "world_list container not found in GUI 'singleplayer'\n";
		return;
	}
	auto& world_list = *world_list_;

	std::vector<fs::path> dirs;
	for(const fs::directory_entry& entry : fs::directory_iterator("worlds"))
	{
		const fs::path& path = entry.path();
		if(!fs::is_regular_file(path / "world"))
		{
			continue;
		}
		dirs.emplace_back(path.filename());
	}
	std::sort(dirs.begin(), dirs.end());
	for(const fs::path& dir : dirs)
	{
		bool bad = false;
		storage::world_file file("worlds" / dir);
		string world_name;
		try
		{
			world_name = file.get_name();
		}
		catch(...)
		{
			bad = true;
			world_name = "(corrupted)";
		}
		if(world_name.empty())
		{
			world_name = "(unnamed)";
		}
		const string dirstr = dir.u8string();
		if(dirstr != world_name)
		{
			world_name += " [" + dirstr + ']';
		}

		auto& btn = world_list.emplace_back<widget::Button>(world_name);

		// TODO: proper theming
		btn.set_border_size(glm::dvec4(2));
		btn.set_border_color(glm::dvec4(1));

		if(bad)
		{
			btn.enabled(false);
		}
		else
		{
			btn.on_click([&g, dir](widget::Button&, const glm::dvec2& /*position*/)
			{
				g.load_world(dir);
			});
		}
	}

	refresh_layout();
}

string singleplayer::type() const
{
	return "singleplayer";
}

}
