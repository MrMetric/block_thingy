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

		void keypress(int key, int scancode, int action, int mods);
		void move(const glm::dvec3& acceleration);
		void step(double delta_time);
		glm::dvec3 apply_movement_input(glm::dvec3 acceleration, double move_speed);
		void reset_position();

		bool can_place_block_at(const Position::BlockInWorld& bwp);

		void jump();
		void toggle_noclip();

		glm::dvec3 pos;
		glm::dvec3 rot;
		glm::dvec3 velocity;

	private:
		bool block_is_at(const double x, const double y, const double z);
		double move_to(double coord, const double move_var, const double offset, Position::BlockInWorld bwp);

		bool key_forward = false;
		bool key_backward = false;
		bool key_left = false;
		bool key_right = false;
		bool on_ground;
		bool do_jump = false;
		bool noclip = false;
};