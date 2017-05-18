#include "Player.hpp"

#include <cmath>

#include <glm/common.hpp>			// glm::sign
#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/vec3.hpp>

#include "Game.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/Enum/Type.hpp"
#include "event/EventManager.hpp"
#include "event/type/Event_enter_block.hpp"
#include "position/BlockInWorld.hpp"

using std::string;

Player::Player
(
	Game& game,
	const string& name
)
:
	name(name),
	reach_distance(16),
	spawn_position(0.5, 1.0, 0.5), // TODO: generate this
	position(spawn_position, [this, &game](glm::dvec3 p)
	{
		p.y += eye_height;
		game.camera.position = p;
	}),
	rotation(glm::dvec3(0), [&game](glm::dvec3 r)
	{
		game.camera.rotation = r;
	}),
	velocity(glm::dvec3(0), [](glm::dvec3 v)
	{
	}),
	game(game),
	abs_offset(0.4),
	eye_height(1.6),
	height(1.8),
	walk_speed(2),
	max_velocity(1)
{
}

void Player::move(const glm::dvec3& acceleration)
{
	const double sinY = std::sin(glm::radians(rotation().y));
	const double cosY = std::cos(glm::radians(rotation().y));

	glm::dvec3 velocity = this->velocity();
	velocity += acceleration;

	if(std::abs(velocity.x) > max_velocity)
	{
		velocity.x = glm::sign(velocity.x) * max_velocity;
	}
	if(std::abs(velocity.z) > max_velocity)
	{
		velocity.z = glm::sign(velocity.z) * max_velocity;
	}

	const glm::dvec3 move_vec
	{
		velocity.x * cosY - velocity.z * sinY,
		velocity.y,
		velocity.z * cosY + velocity.x * sinY,
	};
	glm::dvec3 position = this->position();
	glm::dvec3 new_position = position + move_vec;

	if(flags.noclip)
	{
		this->velocity = velocity;
		this->position = new_position;
		return;
	}

	AABB new_aabb = make_aabb(new_position);
	const Position::BlockInWorld block_pos_old(position);
	auto loop = [this, &move_vec, &new_position, &new_aabb, &block_pos_old](const bool corners)
	{
		Position::BlockInWorld block_pos_offset;
		for(block_pos_offset.y = 0; block_pos_offset.y <= std::floor(height); ++block_pos_offset.y)
		for(block_pos_offset.x = -1; block_pos_offset.x <= 1; ++block_pos_offset.x)
		for(block_pos_offset.z = -1; block_pos_offset.z <= 1; ++block_pos_offset.z)
		{
			bool skip = block_pos_offset.x != 0 && block_pos_offset.z != 0;
			if(corners) skip = !skip;
			if(skip) continue;

			const Position::BlockInWorld block_pos = block_pos_old + block_pos_offset;
			if(!game.world.get_block(block_pos).is_solid())
			{
				continue;
			}
			AABB block_aabb(block_pos);
			if(!new_aabb.collide(block_aabb))
			{
				continue;
			}
			const glm::vec3 direction_sign(block_pos_offset.x, 0, block_pos_offset.z);
			glm::dvec3 offset = new_aabb.offset(block_aabb, direction_sign);
			if(corners)
			{
				// avoid being stuck on corners
				const glm::dvec3 abs_offset = glm::abs(offset);
				if(abs_offset.x > abs_offset.z)
				{
					offset.x = 0;
				}
				else if(abs_offset.z > abs_offset.x)
				{
					offset.z = 0;
				}
			}
			new_position += offset;
			new_aabb = make_aabb(new_position);
		}
	};
	loop(false); // skip corners
	loop(true);  // skip sides
	position = new_position;

	// TODO: use AABBs
	{
		if(move_vec.y < 0)
		{
			const Position::BlockInWorld pos_feet_new(glm::dvec3(position.x, position.y + move_vec.y, position.z));
			const Block::Base& block = game.world.get_block(pos_feet_new);
			if(block.is_solid())
			{
				position.y = pos_feet_new.y + 1;
				if(flags.on_ground)
				{
					velocity.y = 0;
				}
				else
				{
					velocity.y *= -block.bounciness();
					flags.on_ground = true;
				}
			}
			else
			{
				flags.on_ground = false;
			}
		}
		else if(move_vec.y > 0)
		{
			const Position::BlockInWorld pos_head_new(glm::dvec3(position.x, position.y + move_vec.y + height, position.z));
			const Block::Base& block = game.world.get_block(pos_head_new);
			if(block.is_solid())
			{
				position.y = pos_head_new.y - height;
				velocity.y *= -block.bounciness();
			}
			flags.on_ground = false;
		}
		else
		{
			flags.on_ground = true;
		}
	}

	this->position = position;
	this->velocity = velocity;
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

	glm::dvec3 velocity = this->velocity();
	if(flags.noclip)
	{
		velocity *= 0.75;
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
	this->velocity = velocity;

	if(acceleration != glm::dvec3(0))
	{
		const Position::BlockInWorld old_position(position());
		move(acceleration * delta_time);
		const Position::BlockInWorld new_position(position());
		if(new_position != old_position)
		{
			const Block::Base& block = game.world.get_block(new_position);
			if(block.type() != Block::Enum::Type::none
			&& block.type() != Block::Enum::Type::air
			&& !block.is_solid())
			{
				game.event_manager.do_event(Event_enter_block(*this, block));
			}
		}

		set_aabb();
	}
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

	acceleration.x += analog_motion.x * move_speed;
	acceleration.z += analog_motion.y * move_speed;
	analog_motion.x = analog_motion.y = 0.0;

	return acceleration;
}

void Player::set_analog_motion(const glm::dvec2& vec)
{
	analog_motion = vec;
}

void Player::respawn()
{
	rotation = glm::dvec3(0);
	velocity = glm::dvec3(0);

	position = spawn_position;
	// TODO: if spawn is blocked, move to a nearby empty area

	set_aabb();
}

bool Player::can_place_block_at(const Position::BlockInWorld& block_pos)
{
	if(flags.noclip)
	{
		return true;
	}
	AABB block_aabb(block_pos);
	return !aabb.collide(block_aabb);
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

bool Player::get_noclip() const
{
	return flags.noclip;
}

void Player::set_noclip(bool noclip)
{
	flags.noclip = noclip;
}

AABB Player::make_aabb(const glm::dvec3& position)
{
	const glm::dvec3 size(abs_offset, height, abs_offset);
	AABB aabb;
	aabb.min = {position.x - size.x, position.y         , position.z - size.z};
	aabb.max = {position.x + size.x, position.y + size.y, position.z + size.z};
	return aabb;
}

void Player::set_aabb()
{
	aabb = make_aabb(position());
}
