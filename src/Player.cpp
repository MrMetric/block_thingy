#include "Player.hpp"

#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/trigonometric.hpp>	// glm::radians
#include <glm/common.hpp>			// glm::sign

#include "Block.hpp"
#include "Coords.hpp"
#include "Game.hpp"

Player::Player()
	:
	on_ground(false)
{
}

Player::~Player()
{
}

void Player::keypress(int key, int scancode, int action, int mods)
{
	bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
	bool released = (action == GLFW_RELEASE);
	switch(key)
	{
		case GLFW_KEY_W:
		{
			key_forward = pressed;
			break;
		}
		case GLFW_KEY_S:
		{
			key_backward = pressed;
			break;
		}
		case GLFW_KEY_A:
		{
			key_left = pressed;
			break;
		}
		case GLFW_KEY_D:
		{
			key_right = pressed;
			break;
		}
		case GLFW_KEY_R:
		{
			this->reset_position();
			break;
		}
		case GLFW_KEY_SEMICOLON:
		{
			std::ofstream streem("cam_pos");
			streem << this->pos.x << " " << this->pos.y << " " << this->pos.z << " ";
			streem << this->rot.x << " " << this->rot.y << " " << this->rot.z;
			streem.flush();
			break;
		}
		case GLFW_KEY_P:
		{
			std::ifstream streem("cam_pos");
			streem >> this->pos.x;
			streem >> this->pos.y;
			streem >> this->pos.z;
			streem >> Game::instance->cam.rotation.x;
			streem >> Game::instance->cam.rotation.y;
			streem >> Game::instance->cam.rotation.z;
			break;
		}
		case GLFW_KEY_SPACE:
		{
			if(pressed && !this->jump)
			{
				this->jump = true;
			}
			break;
		}
		case GLFW_KEY_L:
		{
			if(pressed)
			{
				this->noclip = !this->noclip;
			}
			break;
		}
		case GLFW_KEY_N: // n for nazi
		{
			if(released)
			{
				if(Game::instance->phys.bwp == nullptr)
				{
					break;
				}
				Position::BlockInWorld bwp = *Game::instance->phys.bwp + Game::instance->phys.face;
				uint16_t nazi[9][9]
				{
					{ 1, 0, 0, 0, 1, 1, 1, 1, 1, },
					{ 1, 0, 0, 0, 1, 0, 0, 0, 0, },
					{ 1, 0, 0, 0, 1, 0, 0, 0, 0, },
					{ 1, 0, 0, 0, 1, 0, 0, 0, 0, },
					{ 1, 1, 1, 1, 1, 1, 1, 1, 1, },
					{ 0, 0, 0, 0, 1, 0, 0, 0, 1, },
					{ 0, 0, 0, 0, 1, 0, 0, 0, 1, },
					{ 0, 0, 0, 0, 1, 0, 0, 0, 1, },
					{ 1, 1, 1, 1, 1, 0, 0, 0, 1, },
				};
				for(bwp_type x = 0; x < 9; ++x)
				{
					for(bwp_type y = 8; y >= 0; --y)
					{
						for(bwp_type z = 0; z < 1; ++z)
						{
							Position::BlockInWorld block_pos(bwp.x + x, bwp.y + y, bwp.z + z);
							if(nazi[y][x] == 0)
							{
								Game::instance->world.set_block(block_pos, nullptr);
							}
							else
							{
								Block* block = new Block(2);
								Game::instance->world.set_block(block_pos, block);
							}
						}
					}
				}
			}
			break;
		}
		default: break;
	}
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
		if(this->jump)
		{
			this->jump = false;
			acceleration.y += 0.30 * sqrt(60 / delta_time);
		}
	}
	else
	{
		if(this->jump && this->on_ground)
		{
			this->jump = false;
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
	if(this->key_forward)
	{
		acceleration.z -= move_speed;
	}
	if(this->key_backward)
	{
		acceleration.z += move_speed;
	}
	if(this->key_left)
	{
		acceleration.x -= move_speed;
	}
	if(this->key_right)
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