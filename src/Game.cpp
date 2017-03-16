#include "Game.hpp"

#include <cmath>
#include <fstream>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <easylogging++/easylogging++.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.hpp"
#include "FPSManager.hpp"
#include "Gfx.hpp"
#include "Player.hpp"
#include "ResourceManager.hpp"
#include "Util.hpp"
#include "World.hpp"
#include "block/Base.hpp"
#include "block/BlockType.hpp"
#include "console/Console.hpp"
#include "console/KeybindManager.hpp"
#include "event/EventManager.hpp"
#include "event/type/Event_window_size_change.hpp"
#include "graphics/RenderWorld.hpp"
#include "graphics/GUI/Base.hpp"
#include "graphics/GUI/Pause.hpp"
#include "graphics/GUI/Play.hpp"
#include "physics/PhysicsUtil.hpp"
#include "physics/RaycastHit.hpp"
#include "position/BlockInChunk.hpp"
#include "position/BlockInWorld.hpp"
#include "position/ChunkInWorld.hpp"
#include "util/key_mods.hpp"

#include "block/Air.hpp"
#include "block/Glass.hpp"
#include "block/Light.hpp"
#include "block/None.hpp"
#include "block/Test.hpp"
#include "block/Teleporter.hpp"
#include "block/Unknown.hpp"

#include "std_make_unique.hpp"

using std::string;
using std::unique_ptr;

Game* Game::instance = nullptr;

Game::Game()
:
	set_instance(this),
	hovered_block(nullptr),
	camera(gfx, event_manager),
	world(block_registry, "worlds/test"),
	player_ptr(world.add_player("test_player")),
	player(*player_ptr),
	console(*this),
	keybinder(console),
	wireframe(false),
	delta_time(0),
	fps(999),
	render_distance(3)
{
	// these must be added first (in this order!) to get the correct IDs
	add_block<Block::None>("none");
	add_block<Block::Air>("air");
	add_block<Block::Unknown>("unknown");

	add_block<Block::Test>("test");
	add_block<Block::Teleporter>("teleporter");
	add_block<Block::Light>("light");
	add_block<Block::Glass>("glass");

	resource_manager.load_blocks(*this);

	if(block_registry.get_extid_map().size() == 0)
	{
		// this should be done when starting a new world, but the design does not work that way yet
		block_registry.reset_extid_map();
	}

	block_type = block_registry.get_id("White");

	gui = std::make_unique<Graphics::GUI::Play>(*this);
	gui->init();

	gfx.hook_events(event_manager);

	add_commands();
	console.run_line("exec binds");

	update_framebuffer_size(gfx.window_size);

	glfwSetJoystickCallback([](int joystick, int event)
	{
		if(joystick == GLFW_JOYSTICK_1 && event == GLFW_DISCONNECTED)
		{
			Game::instance->player.set_analog_motion({ 0, 0 });
		}
	});
}

void Game::draw()
{
	resource_manager.check_updates();

	glViewport(0, 0, gfx.window_size.x, gfx.window_size.y);

	glBindFramebuffer(GL_FRAMEBUFFER, gfx.screen_rt.frame_buffer.get_name());
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	gui->draw();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gfx.buf_rt.frame_buffer.get_name());
	// GL_READ_FRAMEBUFFER is screen_rt.frame_buffer
	glBlitFramebuffer
	(
		0, 0, gfx.window_size.x, gfx.window_size.y,
		0, 0, gfx.window_size.x, gfx.window_size.y,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(gfx.buf_rt.frame_texture.type, gfx.buf_rt.frame_texture.get_name());
	glUseProgram(gfx.screen_shader->get_name());
	gfx.screen_shader->uniform("time", static_cast<float>(world.get_time()));
	gfx.quad_vao.draw(GL_TRIANGLES, 0, 6);

	glViewport(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
	glScissor(3 * gfx.window_size.x / 4, 0, gfx.window_size.x / 4, gfx.window_size.y / 4);
	glEnable(GL_SCISSOR_TEST);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
	// TODO: split stepping and drawing to allow using gui->draw() here
	draw_world(camera.position, camera.rotation);


	glfwSwapBuffers(gfx.window);

	glfwPollEvents();

	if(glfwJoystickPresent(GLFW_JOYSTICK_1))
	{
		int count;

		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		for(int i = 0; i < count; ++i)
		{
			joypress(1, i, buttons[i] != 0);
		}

		auto fix_axis = [](float axis)
		{
			if(std::abs(axis) < 0.1f)
			{
				return 0.0f;
			}
			return axis;
		};

		const float* axises = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);
		player.set_analog_motion({ fix_axis(axises[0]), fix_axis(axises[1]) });
		glm::dvec2 motion(fix_axis(axises[3]), fix_axis(axises[4]));
		joymove(motion);
	}

	std::stringstream ss;
	ss << "Baby's First Voxel Engine | " << fps.getFPS() << " fps";
	ss << " | player.pos(" << glm::to_string(player.position()) << ")";
	Position::BlockInWorld player_block_pos(player.position());
	ss << " | block" << player_block_pos;
	ss << " | chunk" << Position::ChunkInWorld(player_block_pos);
	ss << " | chunkblock" << Position::BlockInChunk(player_block_pos);
	glfwSetWindowTitle(gfx.window, ss.str().c_str());

	delta_time = fps.enforceFPS();
}

void Game::step_world()
{
	player.rotation = camera.rotation;
	world.step(delta_time);
}

void Game::draw_world()
{
	draw_world(camera.position, camera.rotation);
}

void Game::draw_world(const glm::dvec3& position, const glm::dvec3& rotation)
{
	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	gfx.set_camera_view(position, rotation);
	Position::BlockInWorld render_origin(position);
	RenderWorld::draw_world(world, gfx.block_shaders, gfx.matriks, render_origin, render_distance);
	find_hovered_block();

	if(wireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void Game::open_gui(unique_ptr<Graphics::GUI::Base> gui)
{
	if(gui == nullptr)
	{
		LOG(WARNING) << "Tried to open a null GUI";
		return;
	}
	gui->init();
	gui->parent = std::move(this->gui);
	this->gui = std::move(gui);

	double x, y;
	glfwGetCursorPos(gfx.window, &x, &y);
	this->gui->mousemove(x, y);
}

void Game::quit()
{
	glfwSetWindowShouldClose(gfx.window, GL_TRUE);
}

void Game::screenshot(string filename)
{
	if(Util::create_directory("screenshots"))
	{
		filename = "screenshots/" + filename;
	}
	// TODO: check file existence
	LOG(INFO) << "saving screenshot to " << filename;
	const auto width = gfx.window_size.x;
	const auto height = gfx.window_size.y;
	auto pixels = std::make_unique<GLubyte[]>(3 * width * height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	Gfx::write_png_RGB(filename.c_str(), pixels.get(), width, height, true);
}

void Game::update_framebuffer_size(const window_size_t& window_size)
{
	event_manager.do_event(Event_window_size_change(window_size));
}

void Game::keypress(const int key, const int scancode, const int action, const Util::key_mods mods)
{
	gui->keypress(key, scancode, action, mods);
}

void Game::charpress(const char32_t codepoint, const Util::key_mods mods)
{
	gui->charpress(codepoint, mods);
}

void Game::mousepress(const int button, const int action, const Util::key_mods mods)
{
	gui->mousepress(button, action, mods);
}

void Game::mousemove(const double x, const double y)
{
	gui->mousemove(x, y);
}

void Game::joypress(const int joystick, const int button, const bool pressed)
{
	gui->joypress(joystick, button, pressed);
}

void Game::joymove(const glm::dvec2& motion)
{
	gui->joymove(motion);
}

static void add_shader(Game& game, const BlockType t, const string& shader_path)
{
	try
	{
		game.gfx.block_shaders.emplace(t, "shaders/block/" + shader_path);
	}
	catch(const std::runtime_error& e)
	{
		LOG(ERROR) << "shader error:\n" << e.what();
		game.gfx.block_shaders.emplace(t, "shaders/block/default");
	}
}

void Game::add_block(const string& strid, BlockType t)
{
	LOG(DEBUG) << "ID " << static_cast<block_type_id_t>(t) << ": " << strid;
	if(t == BlockType::none || t == BlockType::air)
	{
		return;
	}
	add_shader(*this, t, strid);
}

BlockType Game::add_block_2(const std::string& name, const std::string& shader_path)
{
	BlockType t = block_registry.add<Block::Base>(name);
	LOG(DEBUG) << "ID " << static_cast<block_type_id_t>(t) << ": " << name;
	add_shader(*this, t, shader_path);
	return t;
}

void Game::find_hovered_block()
{
	glm::dvec3 out_origin;
	glm::dvec3 out_direction;
	PhysicsUtil::ScreenPosToWorldRay
	(
		gfx.window_mid,
		gfx.window_size,
		gfx.view_matrix_graphical,
		gfx.projection_matrix,
		out_origin,
		out_direction
	);

	glm::dvec3 offset = gfx.physical_position - gfx.graphical_position;
	hovered_block = PhysicsUtil::raycast(world, out_origin + offset, out_direction, player.reach_distance);
	if(hovered_block != nullptr)
	{
		const glm::dvec4 color = world.get_block(hovered_block->pos).selection_color();
		gfx.draw_cube_outline(hovered_block->pos, color);
	}
}

void Game::add_commands()
{
	#define COMMAND_(name) commands.emplace_back(console, name, [](Game& game
	#define COMMAND(name) COMMAND_(name))
	#define COMMAND_ARGS(name) COMMAND_(name), const std::vector<string>& args)

	COMMAND("save")
	{
		game.world.save();
	});
	COMMAND("quit")
	{
		game.quit();
	});

	COMMAND("break_block")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->pos;
		if(game.world.get_block(pos).type() != BlockType::none) // TODO: breakability check
		{
			game.world.set_block(pos, game.block_registry.make(BlockType::air));
			game.find_hovered_block();
			//event_manager.do_event(Event_break_block(pos, face));
		}
	});
	COMMAND("place_block")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->adjacent();
		if(game.world.get_block(pos).is_replaceable())
		{
			// TODO: check solidity without constructing
			unique_ptr<Block::Base> block = game.block_registry.make(game.block_type);
			if(game.player.can_place_block_at(pos) || !block->is_solid())
			{
				game.world.set_block(pos, std::move(block));
				//event_manager.do_event(Event_place_block(pos, face));
			}
		}
	});
	COMMAND("pick_block")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld pos = game.hovered_block->pos;
		const Block::Base& block = game.world.get_block(pos);
		game.block_type = block.type();
	});

	// TODO: less copy/paste
	COMMAND("+forward")
	{
		game.player.move_forward(true);
	});
	COMMAND("-forward")
	{
		game.player.move_forward(false);
	});
	COMMAND("+backward")
	{
		game.player.move_backward(true);
	});
	COMMAND("-backward")
	{
		game.player.move_backward(false);
	});
	COMMAND("+left")
	{
		game.player.move_left(true);
	});
	COMMAND("-left")
	{
		game.player.move_left(false);
	});
	COMMAND("+right")
	{
		game.player.move_right(true);
	});
	COMMAND("-right")
	{
		game.player.move_right(false);
	});
	COMMAND("jump")
	{
		game.player.jump();
	});
	COMMAND("+use")
	{
		if(game.hovered_block != nullptr)
		{
			game.world.get_block(game.hovered_block->pos).use_start();
		}
	});
	COMMAND("-use")
	{
		// TODO
	});
	COMMAND("+sprint")
	{
		game.player.go_faster(true);
	});
	COMMAND("-sprint")
	{
		game.player.go_faster(false);
	});

	COMMAND("noclip")
	{
		game.player.toggle_noclip();
	});
	COMMAND("respawn")
	{
		game.player.respawn();
		LOG(INFO) << "respawned at " << glm::to_string(game.player.position());
	});

	COMMAND_ARGS("save_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: save_pos <string: filename>";
			return;
		}
		const string save_name = args[0];
		std::ofstream streem(save_name);
		streem.precision(std::numeric_limits<double>::max_digits10);

		const glm::dvec3 pos = game.player.position();
		streem << pos.x << " " << pos.y << " " << pos.z << "\n";

		const glm::dvec3 rot = game.player.rotation();
		streem << rot.x << " " << rot.y << " " << rot.z;

		streem.flush();
		LOG(INFO) << "saved position and rotation to " << save_name;
	});
	COMMAND_ARGS("load_pos")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: load_pos <string: filename>";
			return;
		}
		const string save_name = args[0];
		std::ifstream streem(save_name);

		glm::dvec3 pos;
		streem >> pos.x;
		streem >> pos.y;
		streem >> pos.z;
		game.player.position = pos;
		LOG(INFO) << "set position to " << glm::to_string(pos);

		glm::dvec3 rot;
		streem >> rot.x;
		streem >> rot.y;
		streem >> rot.z;
		game.player.rotation = rot;
		LOG(INFO) << "set rotation to " << glm::to_string(rot);
	});

	COMMAND_ARGS("exec")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: exec <string: filename>";
			return;
		}
		const string name = args[0];
		std::ifstream file("scripts/" + name);
		if(!file.is_open())
		{
			LOG(ERROR) << "script not found: " << name;
			return;
		}
		for(string line; std::getline(file, line); )
		{
			game.console.run_line(line);
		}
	});

	COMMAND_ARGS("cam.rot")
	{
		if(args.size() != 2)
		{
			LOG(ERROR) << "Usage: cam.rot x|y|z <float: degrees>";
			return;
		}
		const string part = args[0];
		const double value = std::stod(args[1]);
		if(part == "x")
		{
			game.camera.rotation.x += value;
		}
		else if(part == "y")
		{
			game.camera.rotation.y += value;
		}
		else if(part == "z")
		{
			game.camera.rotation.z += value;
		}
		else
		{
			LOG(ERROR) << "component name must be x, y, or z";
			return;
		}
		LOG(INFO) << "camera rotation: " << glm::to_string(game.camera.rotation);
	});

	COMMAND_ARGS("screenshot")
	{
		string filename;
		if(args.size() == 0)
		{
			filename = Util::datetime() + ".png";
		}
		else if(args.size() == 1)
		{
			filename = args[0];
		}
		else
		{
			LOG(ERROR) << "Usage: screenshot [string: filename]";
			return;
		}
		try
		{
			game.screenshot(filename);
		}
		catch(const std::runtime_error& e)
		{
			LOG(ERROR) << "error saving screenshot: " << e.what();
		}
	});

	COMMAND("toggle_fullscreen")
	{
		game.gfx.toggle_fullscreen();
	});
	COMMAND("toggle_wireframe")
	{
		game.wireframe = !game.wireframe;
		LOG(INFO) << "wireframe: " << game.wireframe;
	});
	COMMAND("toggle_cull_face")
	{
		game.gfx.toggle_cull_face();
		LOG(INFO) << "cull face: " << game.gfx.cull_face;
	});
	COMMAND_ARGS("fov")
	{
		if(args.size() != 1 || args[0].length() == 0)
		{
			LOG(ERROR) << "Usage: fov <exact number or +- difference>";
			return;
		}

		const string svalue = args[0];
		const double value = std::stod(svalue);
		if(svalue[0] == '+' || svalue[0] == '-')
		{
			game.gfx.fov += value;
		}
		else
		{
			game.gfx.fov = value;
		}

		if(game.gfx.fov < 0)
		{
			game.gfx.fov = std::fmod(game.gfx.fov, 360) + 360;
		}
		else
		{
			game.gfx.fov = std::fmod(game.gfx.fov, 360);
		}
		if(game.gfx.fov == 0) // avoid division by zero
		{
			game.gfx.fov = 360;
		}

		game.gfx.update_projection_matrix();
	});
	COMMAND_ARGS("screen_shader")
	{
		if(args.size() != 1 || args[0].size() == 0)
		{
			LOG(ERROR) << "Usage: screen_shader <path>";
			return;
		}

		game.gfx.set_screen_shader(args[0]);
	});

	COMMAND("render_distance++")
	{
		if(std::numeric_limits<decltype(game.render_distance)>::max() > game.render_distance)
		{
			game.render_distance += 1;
		}
		LOG(INFO) << "render distance: " << game.render_distance;
	});
	COMMAND("render_distance--")
	{
		game.render_distance -= 1;
		if(game.render_distance < 0)
		{
			game.render_distance = 0;
		}
		LOG(INFO) << "render distance: " << game.render_distance;
	});
	COMMAND("reach_distance++")
	{
		game.player.reach_distance += 1;
		LOG(INFO) << "reach distance: " << game.player.reach_distance;
	});
	COMMAND("reach_distance--")
	{
		game.player.reach_distance -= 1;
		LOG(INFO) << "reach distance: " << game.player.reach_distance;
	});

	COMMAND("block_type++")
	{
		block_type_id_t i = static_cast<block_type_id_t>(game.block_type);
		i = (i + 1) % Block::MAX_ID;
		if(i < 2)
		{
			i = 2;
		}
		game.block_type = static_cast<BlockType>(i);
		LOG(DEBUG) << "block type: " << i;
	});
	COMMAND("block_type--")
	{
		block_type_id_t i = static_cast<block_type_id_t>(game.block_type);
		if(i == 2)
		{
			i = Block::MAX_ID - 1;
		}
		else
		{
			i = (i - 1) % Block::MAX_ID;
		}
		game.block_type = static_cast<BlockType>(i);
		LOG(DEBUG) << "block type: " << i;
	});

	COMMAND("nazi")
	{
		if(game.hovered_block == nullptr)
		{
			return;
		}

		const Position::BlockInWorld start_pos = game.hovered_block->adjacent();
		const Position::BlockInWorld::value_type ysize = 9;
		const Position::BlockInWorld::value_type xsize = 9;
		block_type_id_t nazi[ysize][xsize]
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
		Position::BlockInWorld pos;
		for(pos.x = 0; pos.x < xsize; ++pos.x)
		{
			for(pos.y = ysize - 1; pos.y >= 0; --pos.y)
			{
				for(pos.z = 0; pos.z < 1; ++pos.z)
				{
					const BlockType type = static_cast<BlockType>(nazi[pos.y][pos.x]);
					game.world.set_block(pos + start_pos, game.block_registry.make(type));
				}
			}
		}
	});

	COMMAND_ARGS("open_gui")
	{
		if(args.size() != 1)
		{
			LOG(ERROR) << "Usage: open_gui <GUI name>";
			return;
		}
		const string name = args[0];
		unique_ptr<Graphics::GUI::Base> gui;
		if(name == "pause")
		{
			gui = std::make_unique<Graphics::GUI::Pause>(game);
		}
		else if(name == "console")
		{
			// TODO
		}
		else
		{
			LOG(ERROR) << "No such GUI: " << name;
			return;
		}
		game.open_gui(std::move(gui));
	});
	COMMAND("close_gui")
	{
		game.gui->close();
	});

	#undef COMMAND_ARGS
	#undef COMMAND
	#undef COMMAND_
}
