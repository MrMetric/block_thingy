#include "command_test.hpp"

#include "Console.hpp"
#include "../Block.hpp"
#include "../BlockType.hpp"
#include "../Game.hpp"
#include "../World.hpp"
#include "../physics/RaytraceHit.hpp"
#include "../position/BlockInWorld.hpp"

void add_test_commands(Game* game)
{
	game->console.add_command("nazi", {[game]()
	{
		if(game->hovered_block == nullptr)
		{
			return;
		}
		Position::BlockInWorld start_pos = game->hovered_block->adjacent();
		block_type_id_t nazi[9][9]
		{
			{ 2, 1, 1, 1, 2, 2, 2, 2, 2, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 1, 1, 1, 2, 1, 1, 1, 1, },
			{ 2, 2, 2, 2, 2, 2, 2, 2, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 1, 1, 1, 1, 2, 1, 1, 1, 2, },
			{ 2, 2, 2, 2, 2, 1, 1, 1, 2, },
		};
		for(BlockInWorld_type x = 0; x < 9; ++x)
		{
			for(BlockInWorld_type y = 8; y >= 0; --y)
			{
				for(BlockInWorld_type z = 0; z < 1; ++z)
				{
					Position::BlockInWorld block_pos(x, y, z);
					block_pos += start_pos;
					block_type_id_t block_id = nazi[y][x];
					game->world.set_block(block_pos, Block(block_id));
				}
			}
		}
	}});
}
