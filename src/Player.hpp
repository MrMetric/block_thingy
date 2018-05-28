#pragma once

#include <memory>
#include <optional>
#include <string>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "fwd/game.hpp"
#include "block/block.hpp"
#include "fwd/graphics/GUI/Base.hpp"
#include "physics/AABB.hpp"
#include "physics/raycast_hit.hpp"
#include "fwd/position/block_in_world.hpp"
#include "util/Property.hpp"
#include "fwd/world/world.hpp"

namespace block_thingy {

class Player
{
public:
	Player
	(
		game&,
		std::string name
	);

	Player(Player&&) = delete;
	Player(const Player&) = delete;
	Player& operator=(Player&&) = delete;
	Player& operator=(const Player&) = delete;

	const std::string name;
	double reach_distance;

	void move(world::world&, const glm::dvec3& acceleration);
	void step(world::world&, double delta_time);
	glm::dvec3 apply_movement_input(glm::dvec3 acceleration, double move_speed);
	void set_analog_motion(const glm::dvec2&);
	void respawn();

	bool can_place_block_at(const position::block_in_world&);

	void move_forward(bool);
	void move_backward(bool);
	void move_left(bool);
	void move_right(bool);
	void go_faster(bool);
	void jump();

	void toggle_noclip();
	bool get_noclip() const;
	void set_noclip(bool);

	void open_gui(std::unique_ptr<graphics::gui::Base>);

	glm::dvec3 spawn_position;
	util::property<glm::dvec3> position;
	util::property<glm::dvec3> rotation;
	util::property<glm::dvec3> velocity;

	std::optional<block_t> copied_block;
	std::optional<physics::raycast_hit> hovered_block;

private:
	game& g;

	const double abs_offset;
	physics::AABB aabb;
	physics::AABB make_aabb(const glm::dvec3& position);
	void set_aabb();
	double eye_height;
	double height;
	double walk_speed;
	double max_velocity;
	glm::dvec2 analog_motion;

	struct
	{
		bool moving_forward = false;
		bool moving_backward = false;
		bool moving_left = false;
		bool moving_right = false;
		bool going_faster = false;
		bool on_ground = false;
		bool do_jump = false;
		bool noclip = false;
	} flags;
};

}
