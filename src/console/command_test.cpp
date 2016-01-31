#include "command_test.hpp"

#include "../Block.hpp"
#include "../Coords.hpp"
#include "../Game.hpp"
#include "../physics/RaytraceHit.hpp"

void add_test_commands(Game* game)
{
	game->console.add_command("nazi", [game](std::vector<std::string> args)
	{
		if(game->hovered_block == nullptr)
		{
			return;
		}
		Position::BlockInWorld bwp = game->hovered_block->adjacent();
		block_id_type nazi[9][9]
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
		for(BlockInWorld_type x = 0; x < 9; ++x)
		{
			for(BlockInWorld_type y = 8; y >= 0; --y)
			{
				for(BlockInWorld_type z = 0; z < 1; ++z)
				{
					Position::BlockInWorld block_pos(bwp.x + x, bwp.y + y, bwp.z + z);
					block_id_type block_id = nazi[y][x];
					if(block_id == 0)
					{
						game->world.set_block(block_pos, nullptr);
					}
					else
					{
						Block* block = new Block(block_id);
						game->world.set_block(block_pos, block);
					}
				}
			}
		}
	});
}