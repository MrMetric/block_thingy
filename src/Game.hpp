#pragma once

#include <cassert>
#include <memory>
#include <experimental/propagate_const>
#include <string>

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
#include "position/ChunkInWorld.hpp"
#include "types/window_size_t.hpp"
#include "fwd/util/char_press.hpp"
#include "util/filesystem.hpp"
#include "fwd/util/key_press.hpp"
#include "fwd/util/mouse_press.hpp"

class Game
{
public:
	Game();
	~Game();

	Game(Game&&) = delete;
	Game(const Game&) = delete;
	void operator=(const Game&) = delete;

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
	void open_gui(std::unique_ptr<Graphics::GUI::Base>);
	void close_gui();
	void quit();

	void screenshot(fs::path) const;
	double get_fps() const;
	Position::ChunkInWorld::value_type get_render_distance() const;

	void update_framebuffer_size(const window_size_t&);
	void keypress(const Util::key_press&);
	void charpress(const Util::char_press&);
	void mousepress(const Util::mouse_press&);
	void mousemove(double x, double y);
	void joypress(int joystick, int button, bool pressed);
	void joymove(const glm::dvec2& motion);

	std::shared_ptr<Block::Base> copied_block;
	std::unique_ptr<RaycastHit> hovered_block;

	ResourceManager resource_manager;

	// event_manager must be initialized before others!
	EventManager event_manager;
	Gfx gfx;

	Camera camera;
	Block::BlockRegistry block_registry; // must be initialized before world
	World world;
	std::shared_ptr<Player> player_ptr;
	Player& player;
	KeybindManager keybinder;
	std::unique_ptr<Graphics::GUI::Base> gui;

private:
	Position::ChunkInWorld::value_type render_distance;

	struct impl;
	std::experimental::propagate_const<std::unique_ptr<impl>> pImpl;
};
