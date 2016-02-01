#include "Player.hpp"

#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/common.hpp>			// glm::sign

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
	double sinY = sin(glm::radians(this->rot.y));
	double cosY = cos(glm::radians(this->rot.y));
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
		this->pos.x = this->move_to(this->pos.x, moveX, offset, Position::BlockInWorld(this->pos.x + moveX + offset, this->pos.y, this->pos.z));
	}

	{
		double moveZ = this->velocity.z * cosY + this->velocity.x * sinY;
		double offset = moveZ < 0 ? -abs_offset : abs_offset;
		this->pos.z = this->move_to(this->pos.z, moveZ, offset, Position::BlockInWorld(this->pos.x, this->pos.y, this->pos.z + moveZ + offset));
	}

	{
		double moveY = this->velocity.y;
		if(moveY < 0)
		{
			Position::BlockInWorld pos_feet_new(glm::dvec3(this->pos.x, this->pos.y + moveY, this->pos.z));
			//Game::instance->phys.draw_outline(pos_feet_new.x, pos_feet_new.y, pos_feet_new.z, glm::vec4(0, 0, 1, 1));
			if(Game::instance->world.get_block(pos_feet_new) != nullptr)
			{
				this->pos.y = pos_feet_new.y + 1;
				this->velocity.y = 0;
				this->on_ground = true;
			}
			else
			{
				this->pos.y += moveY;
				this->on_ground = false;
			}
		}
		else
		{
			this->pos.y += moveY;
			this->on_ground = false;
		}
	}
}

void Player::step(double delta_time)
{
	glm::dvec3 acceleration;
	acceleration.y -= 0.5; // gravity

	this->rot = Game::instance->cam.rotation;

	acceleration = apply_movement_input(acceleration, this->walk_speed);

	if(this->noclip)
	{
		this->velocity.x *= 0.75;
		this->velocity.y *= 0.75;
		this->velocity.z *= 0.75;
		if(this->do_jump)
		{
			this->do_jump = false;
			acceleration.y += 0.30 * sqrt(60 / delta_time);
		}
	}
	else
	{
		if(this->do_jump && this->on_ground)
		{
			this->do_jump = false;
			acceleration.y += 0.15 * sqrt(60 / delta_time);
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

	acceleration *= delta_time;
	this->move(acceleration);
	Game::instance->cam.position = this->pos;
	Game::instance->cam.position.y += this->eye_height;
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

void Player::reset_position()
{
	this->pos.x = this->pos.y = this->pos.z = 0;
	this->rot.x = this->rot.y = this->rot.z = 0;

	while(!this->block_is_at(this->pos.x, this->pos.y, this->pos.z))
	{
		--this->pos.y;
	}
	while(this->block_is_at(this->pos.x, this->pos.y, this->pos.z))
	{
		++this->pos.y;
	}
	++this->pos.y;

	this->velocity.x = this->velocity.y = this->velocity.z = 0;
}

bool Player::can_place_block_at(const Position::BlockInWorld& bwp)
{
	Position::BlockInWorld pos0(this->pos.x, this->pos.y, this->pos.z);
	if(bwp == pos0)
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
	if(!this->do_jump)
	{
		this->do_jump = true;
	}
}

void Player::toggle_noclip()
{
	this->noclip = !this->noclip;
}

bool Player::block_is_at(const double x, const double y, const double z)
{
	Position::BlockInWorld block_pos(glm::dvec3(x, y, z));
	return (Game::instance->world.get_block(block_pos) != nullptr);
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