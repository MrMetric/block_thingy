#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "Gfx.hpp"
#include "fwd/Player.hpp"
#include "resource_manager.hpp"
#include "block/BlockRegistry.hpp"
#include "fwd/block/enums/type.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "graphics/camera.hpp"
#include "graphics/GUI/Base.hpp"
#include "fwd/input/char_press.hpp"
#include "fwd/input/joy_press.hpp"
#include "fwd/input/key_press.hpp"
#include "fwd/input/mouse_press.hpp"
#include "physics/raycast_hit.hpp"
#include "shim/propagate_const.hpp"
#include "types/window_size_t.hpp"
#include "util/filesystem.hpp"
#include "world/world.hpp"

namespace block_thingy {

class game;

namespace detail {

struct gui_maker_base
{
	virtual ~gui_maker_base();

	virtual std::unique_ptr<graphics::gui::Base> make(game&) const = 0;
};

template<typename T>
struct gui_maker : gui_maker_base
{
	std::unique_ptr<graphics::gui::Base> make(game& g) const
	{
		return std::make_unique<T>(g);
	}
};

}

class game
{
public:
	game();
	~game();

	game(game&&) = delete;
	game(const game&) = delete;
	game& operator=(game&&) = delete;
	game& operator=(const game&) = delete;

	static game* instance;
private:
	class set_instance
	{
		public:
			set_instance(game* ptr)
			{
				assert(game::instance == nullptr);
				game::instance = ptr;
			}
	} set_instance;

public:
	void draw();
	void step();
	void draw_world();
	void draw_world
	(
		const glm::dvec3& cam_position,
		const glm::dvec3& cam_rotation,
		const glm::dmat4& projection_matrix
	);
	void draw_world
	(
		const glm::dvec3& cam_position,
		const glm::dmat4& cam_rotation,
		const glm::dmat4& projection_matrix
	);

	template<typename T>
	void register_gui(const std::string& type)
	{
		register_gui(type, std::make_unique<detail::gui_maker<T>>());
	}
	std::unique_ptr<graphics::gui::Base> make_gui(const std::string& type);
	void open_gui(std::unique_ptr<graphics::gui::Base>);
	void close_gui();
	void quit();

	void load_world(fs::path);
	void screenshot(fs::path) const;
	double get_fps() const;
	uint64_t get_global_ticks() const;
	double get_global_time() const;

	/**
	 * @return The amount of chunks considered for drawing and the amount of chunks drawn in the last frame
	 */
	std::tuple<uint64_t, uint64_t> get_draw_stats() const;

	void update_framebuffer_size(const window_size_t&);
	void keypress(const util::key_press&);
	void charpress(const util::char_press&);
	void mousepress(const util::mouse_press&);
	void mousemove(const glm::dvec2& position);
	void joypress(const input::joy_press&);
	void joymove(const glm::dvec2& offset);

	std::shared_ptr<block::base> copied_block;
	std::optional<physics::raycast_hit> hovered_block;

	block_thingy::resource_manager resource_manager;

	// event_manager must be initialized before others!
	EventManager event_manager;
	Gfx gfx;

	graphics::camera camera;
	block::BlockRegistry block_registry; // must be initialized before world
	std::shared_ptr<world::world> world;
	std::shared_ptr<Player> player;
	KeybindManager keybinder;
	std::unique_ptr<graphics::gui::Base> gui;

private:
	void register_gui(const std::string& type, std::unique_ptr<detail::gui_maker_base>);

	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
