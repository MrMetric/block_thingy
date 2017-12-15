#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

#include "Camera.hpp"
#include "Gfx.hpp"
#include "fwd/Player.hpp"
#include "ResourceManager.hpp"
#include "World.hpp"
#include "block/BlockRegistry.hpp"
#include "fwd/block/Enum/Type.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "graphics/GUI/Base.hpp"
#include "physics/RaycastHit.hpp"
#include "shim/propagate_const.hpp"
#include "types/window_size_t.hpp"
#include "fwd/util/char_press.hpp"
#include "util/filesystem.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"

namespace block_thingy {

class Game
{
public:
	Game();
	~Game();

	Game(Game&&) = delete;
	Game(const Game&) = delete;
	Game& operator=(Game&&) = delete;
	Game& operator=(const Game&) = delete;

	static Game* instance;
private:
	class set_instance
	{
		public:
			set_instance(Game* ptr)
			{
				assert(Game::instance == nullptr);
				Game::instance = ptr;
			}
	} set_instance;

public:
	void draw();
	void step_world();
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
	void open_gui(std::unique_ptr<graphics::gui::Base>);
	void close_gui();
	void quit();

	void screenshot(fs::path) const;
	double get_fps() const;

	/**
	 * @return The amount of chunks considered for drawing and the amount of chunks drawn in the last frame
	 */
	std::tuple<uint64_t, uint64_t> get_draw_stats() const;

	void update_framebuffer_size(const window_size_t&);
	void keypress(const util::key_press&);
	void charpress(const util::char_press&);
	void mousepress(const util::mouse_press&);
	void mousemove(double x, double y);
	void joypress(int joystick, int button, bool pressed);
	void joymove(const glm::dvec2& motion);

	std::shared_ptr<block::Base> copied_block;
	std::optional<physics::RaycastHit> hovered_block;

	ResourceManager resource_manager;

	// event_manager must be initialized before others!
	EventManager event_manager;
	Gfx gfx;

	Camera camera;
	block::BlockRegistry block_registry; // must be initialized before world
	World world;
	std::shared_ptr<Player> player_ptr;
	Player& player;
	KeybindManager keybinder;
	std::unique_ptr<graphics::gui::Base> gui;

private:
	struct impl;
	std::propagate_const<std::unique_ptr<impl>> pImpl;
};

}
