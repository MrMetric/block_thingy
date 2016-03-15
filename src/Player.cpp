#include "Player.hpp"

#include <cmath>

#include <glm/common.hpp>			// glm::sign
#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/vec3.hpp>

#include "Block.hpp"
#include "Game.hpp"
#include "position/BlockInWorld.hpp"

Player::Player()
	:
	abs_offset(0.4),
	eye_height(1.7),
	walk_speed(2),
	max_velocity(1)
{
}

Player::Player(BinaryReader& reader)
	:
	Player()
{
	deserialize(reader);
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
				flags.on_ground = true;
			}
			else
			{
				position.y += moveY;
				flags.on_ground = false;
			}
		}
		else
		{
			position.y += moveY;
			flags.on_ground = false;
		}
	}
}

void Player::step(const double delta_time)
{
	glm::dvec3 acceleration;
	acceleration.y -= 0.5; // gravity

	double move_speed = walk_speed;
	if(flags.going_faster)
	{
		move_speed *= 4;
	}
	acceleration = apply_movement_input(acceleration, move_speed);

	if(flags.noclip)
	{
		velocity.x *= 0.75;
		velocity.y *= 0.75;
		velocity.z *= 0.75;
		acceleration.y = 0;
		if(flags.do_jump)
		{
			acceleration.y += 18;
		}
	}
	else
	{
		if(flags.do_jump && flags.on_ground)
		{
			acceleration.y += 9;
		}

		// friction
		if(flags.on_ground)
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
	flags.do_jump = false;

	move(acceleration * delta_time);
}

glm::dvec3 Player::apply_movement_input(glm::dvec3 acceleration, const double move_speed)
{
	if(flags.moving_forward)
	{
		acceleration.z -= move_speed;
	}
	if(flags.moving_backward)
	{
		acceleration.z += move_speed;
	}
	if(flags.moving_left)
	{
		acceleration.x -= move_speed;
	}
	if(flags.moving_right)
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

bool Player::can_place_block_at(const Position::BlockInWorld& block_pos)
{
	Position::BlockInWorld pos0(position.x, position.y, position.z);
	Position::BlockInWorld pos1(position.x, position.y + 1, position.z);
	if(block_pos == pos0 || block_pos == pos1)
	{
		return false;
	}
	return true;
}

void Player::move_forward(const bool do_that)
{
	flags.moving_forward = do_that;
}

void Player::move_backward(const bool do_that)
{
	flags.moving_backward = do_that;
}

void Player::move_left(const bool do_that)
{
	flags.moving_left = do_that;
}

void Player::move_right(const bool do_that)
{
	flags.moving_right = do_that;
}

void Player::go_faster(const bool do_that)
{
	flags.going_faster = do_that;
}

void Player::jump()
{
	flags.do_jump = true;
}

void Player::toggle_noclip()
{
	flags.noclip = !flags.noclip;
}

bool Player::block_is_at(const double x, const double y, const double z)
{
	const Position::BlockInWorld block_pos(x, y, z);
	const Block& block = Game::instance->world.get_block_const(block_pos);
	return block.is_solid();
}

double Player::move_to(double coord, const double move_var, const double offset, const Position::BlockInWorld& block_pos)
{
	if(flags.noclip)
	{
		return coord + move_var;
	}

	if(block_is_at(block_pos.x, block_pos.y, block_pos.z) || block_is_at(block_pos.x, block_pos.y + 1, block_pos.z))
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

void Player::serialize(BinaryWriter& writer)
{
	writer << position.x << position.y << position.z;
	writer << rotation.x << rotation.y << rotation.z;
	writer << velocity.x << velocity.y << velocity.z;
	writer << flags.on_ground;
	writer << flags.noclip;
}

void Player::deserialize(BinaryReader& reader)
{
	reader >> position.x;
	reader >> position.y;
	reader >> position.z;

	// TODO: the rotation must be set in Camera, not here (same as TODO in respawn)
	reader >> rotation.x;
	reader >> rotation.y;
	reader >> rotation.z;

	reader >> velocity.x;
	reader >> velocity.y;
	reader >> velocity.z;

	reader >> flags.on_ground;
	reader >> flags.noclip;
}
