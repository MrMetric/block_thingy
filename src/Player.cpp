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
	double sinY = std::sin(glm::radians(this->rotation.y));
	double cosY = std::cos(glm::radians(this->rotation.y));
	this->velocity += acceleration;

	if(std::abs(this->velocity.x) > this->max_velocity)
	{
		this->velocity.x = glm::sign(this->velocity.x) * this->max_velocity;
	}
	if(std::abs(this->velocity.z) > this->max_velocity)
	{
		this->velocity.z = glm::sign(this->velocity.z) * this->max_velocity;
	}

	{
		double moveX = this->velocity.x * cosY - this->velocity.z * sinY;
		double offset = moveX < 0 ? -abs_offset : abs_offset;
		this->position.x = this->move_to(this->position.x, moveX, offset, Position::BlockInWorld(this->position.x + moveX + offset, this->position.y, this->position.z));
	}

	{
		double moveZ = this->velocity.z * cosY + this->velocity.x * sinY;
		double offset = moveZ < 0 ? -abs_offset : abs_offset;
		this->position.z = this->move_to(this->position.z, moveZ, offset, Position::BlockInWorld(this->position.x, this->position.y, this->position.z + moveZ + offset));
	}

	{
		double moveY = this->velocity.y;
		if(moveY < 0)
		{
			Position::BlockInWorld pos_feet_new(glm::dvec3(this->position.x, this->position.y + moveY, this->position.z));
			if(Game::instance->world.get_block(pos_feet_new).type() != 0)
			{
				this->position.y = pos_feet_new.y + 1;
				this->velocity.y = 0;
				this->on_ground = true;
			}
			else
			{
				this->position.y += moveY;
				this->on_ground = false;
			}
		}
		else
		{
			this->position.y += moveY;
			this->on_ground = false;
		}
	}
}

void Player::step(const double delta_time)
{
	glm::dvec3 acceleration;
	acceleration.y -= 0.5; // gravity

	acceleration = this->apply_movement_input(acceleration, this->walk_speed);

	if(this->noclip)
	{
		this->velocity.x *= 0.75;
		this->velocity.y *= 0.75;
		this->velocity.z *= 0.75;
		acceleration.y = 0;
		if(this->do_jump)
		{
			acceleration.y += 18;
		}
	}
	else
	{
		if(this->do_jump && this->on_ground)
		{
			acceleration.y += 9;
		}

		// friction
		if(this->on_ground)
		{
			this->velocity.x *= 0.75;
			this->velocity.z *= 0.75;
		}
		else
		{
			this->velocity.x *= 0.99;
			this->velocity.z *= 0.99;

			// no changing your direction in air!
			acceleration.x = 0;
			acceleration.z = 0;
		}
	}
	this->do_jump = false;

	this->move(acceleration * delta_time);
}

glm::dvec3 Player::apply_movement_input(glm::dvec3 acceleration, double move_speed)
{
	if(this->moving_forward)
	{
		acceleration.z -= move_speed;
	}
	if(this->moving_backward)
	{
		acceleration.z += move_speed;
	}
	if(this->moving_left)
	{
		acceleration.x -= move_speed;
	}
	if(this->moving_right)
	{
		acceleration.x += move_speed;
	}
	return acceleration;
}

void Player::respawn()
{
	this->position.x = this->position.y = this->position.z = 0;
	this->rotation.x = this->rotation.y = this->rotation.z = 0; // TODO: improve design (this needs to be set in the camera, not here)

	while(!this->block_is_at(this->position.x, this->position.y, this->position.z))
	{
		--this->position.y;
	}
	while(this->block_is_at(this->position.x, this->position.y, this->position.z))
	{
		++this->position.y;
	}

	this->velocity.x = this->velocity.y = this->velocity.z = 0;
}

bool Player::can_place_block_at(const Position::BlockInWorld& bwp)
{
	Position::BlockInWorld pos0(this->position.x, this->position.y, this->position.z);
	Position::BlockInWorld pos1(this->position.x, this->position.y + 1, this->position.z);
	if(bwp == pos0 || bwp == pos1)
	{
		return false;
	}
	return true;
}

void Player::move_forward(bool do_that)
{
	this->moving_forward = do_that;
}

void Player::move_backward(bool do_that)
{
	this->moving_backward = do_that;
}

void Player::move_left(bool do_that)
{
	this->moving_left = do_that;
}

void Player::move_right(bool do_that)
{
	this->moving_right = do_that;
}

void Player::jump()
{
	this->do_jump = true;
}

void Player::toggle_noclip()
{
	this->noclip = !this->noclip;
}

bool Player::block_is_at(const double x, const double y, const double z)
{
	Position::BlockInWorld block_pos(x, y, z);
	return (Game::instance->world.get_block(block_pos).type() != 0);
}

double Player::move_to(double coord, const double move_var, const double offset, Position::BlockInWorld bwp)
{
	if(this->noclip)
	{
		return coord + move_var;
	}

	if(this->block_is_at(bwp.x, bwp.y, bwp.z) || this->block_is_at(bwp.x, bwp.y + 1, bwp.z))
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