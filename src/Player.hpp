#pragma once

#include <glm/vec3.hpp>

namespace Position
{
	struct BlockInWorld;
}

class Player
{
	public:
		Player();
		virtual ~Player();

		const double abs_offset = 0.4;

		double eye_height = 1.7;
		double walk_speed = 2;
		double max_velocity = 1;

		void move(const glm::dvec3& acceleration);
		void step(double delta_time);
		glm::dvec3 apply_movement_input(glm::dvec3 acceleration, double move_speed);
		void respawn();

		bool can_place_block_at(const Position::BlockInWorld&);

		void move_forward(bool);
		void move_backward(bool);
		void move_left(bool);
		void move_right(bool);
		void jump();
		void toggle_noclip();

		glm::dvec3 position;
		glm::dvec3 rotation;
		glm::dvec3 velocity;

	private:
		bool block_is_at(double x, double y, double z);
		double move_to(double coord, double move_var, double offset, const Position::BlockInWorld&);

		bool moving_forward = false;
		bool moving_backward = false;
		bool moving_left = false;
		bool moving_right = false;
		bool on_ground;
		bool do_jump = false;
		bool noclip = false;
};
