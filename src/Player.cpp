#include "Player.hpp"

#include <cmath>

#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/common.hpp>			// glm::sign

#include "Block.hpp"
#include "Game.hpp"

Player::Player()
	:
	on_ground(false)
{
}

Player::~Player()
{
}

void Player::move(const glm::dvec3& acceleration)
{
	double sinY = std::sin(glm::radians(rotation.y));
	double cosY = std::cos(glm::radians(rotation.y));
	velocity += acceleration;

	if(std::abs(velocity.x) > max_velocity)
	{
		velocity.x = glm::sign(velocity.x) * max_velocity;
	}
	if(std::abs(velocity.z) > max_velocity)
	{
		velocity.z = glm::sign(velocity.z) * max_velocity;
	}

	{
		double moveX = velocity.x * cosY - velocity.z * sinY;
		double offset = moveX < 0 ? -abs_offset : abs_offset;
		position.x = move_to(position.x, moveX, offset, Position::BlockInWorld(position.x + moveX + offset, position.y, position.z));
	}

	{
		double moveZ = velocity.z * cosY + velocity.x * sinY;
		double offset = moveZ < 0 ? -abs_offset : abs_offset;
		position.z = move_to(position.z, moveZ, offset, Position::BlockInWorld(position.x, position.y, position.z + moveZ + offset));
	}

	{
		double moveY = velocity.y;
		if(moveY < 0)
		{
			Position::BlockInWorld pos_feet_new(glm::dvec3(position.x, position.y + moveY, position.z));
			if(block_is_at(pos_feet_new.x, pos_feet_new.y, pos_feet_new.z))
			{
				position.y = pos_feet_new.y + 1;
				velocity.y = 0;
				on_ground = true;
			}
			else
			{
				position.y += moveY;
				on_ground = false;
			}
		}
		else
		{
			position.y += moveY;
			on_ground = false;
		}
	}
}

void Player::step(const double delta_time)
{
	glm::dvec3 acceleration;
	acceleration.y -= 0.5; // gravity

	acceleration = apply_movement_input(acceleration, walk_speed);

	if(noclip)
	{
		velocity.x *= 0.75;
		velocity.y *= 0.75;
		velocity.z *= 0.75;
		acceleration.y = 0;
		if(do_jump)
		{
			acceleration.y += 18;
		}
	}
	else
	{
		if(do_jump && on_ground)
		{
			acceleration.y += 9;
		}

		// friction
		if(on_ground)
		{
			velocity.x *= 0.75;
			velocity.z *= 0.75;
		}
		else
		{
			velocity.x *= 0.99;
			velocity.z *= 0.99;

			// no changing your direction in air!
			acceleration.x = 0;
			acceleration.z = 0;
		}
	}
	do_jump = false;

	move(acceleration * delta_time);
}

glm::dvec3 Player::apply_movement_input(glm::dvec3 acceleration, double move_speed)
{
	if(moving_forward)
	{
		acceleration.z -= move_speed;
	}
	if(moving_backward)
	{
		acceleration.z += move_speed;
	}
	if(moving_left)
	{
		acceleration.x -= move_speed;
	}
	if(moving_right)
	{
		acceleration.x += move_speed;
	}
	return acceleration;
}

void Player::respawn()
{
	position.x = position.y = position.z = 0;
	rotation.x = rotation.y = rotation.z = 0; // TODO: improve design (this needs to be set in the camera, not here)

	while(!block_is_at(position.x, position.y, position.z))
	{
		--position.y;
	}
	while(block_is_at(position.x, position.y, position.z))
	{
		++position.y;
	}

	velocity.x = velocity.y = velocity.z = 0;
}

bool Player::can_place_block_at(const Position::BlockInWorld& bwp)
{
	Position::BlockInWorld pos0(position.x, position.y, position.z);
	Position::BlockInWorld pos1(position.x, position.y + 1, position.z);
	if(bwp == pos0 || bwp == pos1)
	{
		return false;
	}
	return true;
}

void Player::move_forward(bool do_that)
{
	moving_forward = do_that;
}

void Player::move_backward(bool do_that)
{
	moving_backward = do_that;
}

void Player::move_left(bool do_that)
{
	moving_left = do_that;
}

void Player::move_right(bool do_that)
{
	moving_right = do_that;
}

void Player::jump()
{
	do_jump = true;
}

void Player::toggle_noclip()
{
	noclip = !noclip;
}

bool Player::block_is_at(const double x, const double y, const double z)
{
	Position::BlockInWorld block_pos(x, y, z);
	return Game::instance->world.get_block(block_pos).is_solid();
}

double Player::move_to(double coord, const double move_var, const double offset, Position::BlockInWorld bwp)
{
	if(noclip)
	{
		return coord + move_var;
	}

	if(block_is_at(bwp.x, bwp.y, bwp.z) || block_is_at(bwp.x, bwp.y + 1, bwp.z))
	{
		if(move_var > 0)
		{
			coord = std::ceil(coord);
		}
		else
		{
			coord = std::floor(coord);
		}
		coord -= offset;
	}
	else
	{
		coord += move_var;
	}
	return coord;
}
