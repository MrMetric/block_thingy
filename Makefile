CC = clang
CXX = clang++
LD = clang++

CFLAGS = \
	-march=native \
	-isystem lib \
	-Weverything \
	-Wno-missing-braces \
	-Wno-missing-prototypes \
	-Wno-padded \
	-Werror=extra-tokens \
	-Werror=invalid-pp-token \
	-Werror=return-type \
	-Werror=unknown-warning-option \
	-Werror=unknown-pragmas \

CXXFLAGS = $(CFLAGS) \
	-std=c++14 \
	-fexceptions \
	-Isrc \
	-Wno-c++98-compat-pedantic \
	-Wno-shadow \
	-Werror=delete-incomplete \
	-Werror=deprecated \
	-DUSE_LIBPNG \

LDFLAGS = \
	`pkg-config --static --libs glfw3` \
	-lpng \

CFLAGS_DEBUG = $(CFLAGS) -g
CXXFLAGS_DEBUG = $(CXXFLAGS) -g -fsanitize=undefined,integer -DCOOL_DEBUG_STUFF
LDFLAGS_DEBUG = $(LDFLAGS) -fsanitize=undefined,integer
OBJDIR_DEBUG = obj/Debug
OUT_DEBUG = bin/block_thingy_d

CFLAGS_RELEASE = $(CFLAGS) -O3
CXXFLAGS_RELEASE = $(CXXFLAGS) -fomit-frame-pointer -O3 -g
LDFLAGS_RELEASE = $(LDFLAGS)
OBJDIR_RELEASE = obj/Release
OUT_RELEASE = bin/block_thingy_r

OBJ_DEBUG = \
	$(OBJDIR_DEBUG)/glad.o \
	$(OBJDIR_DEBUG)/src/Block.o \
	$(OBJDIR_DEBUG)/src/Camera.o \
	$(OBJDIR_DEBUG)/src/Cube.o \
	$(OBJDIR_DEBUG)/src/Game.o \
	$(OBJDIR_DEBUG)/src/Gfx.o \
	$(OBJDIR_DEBUG)/src/main.o \
	$(OBJDIR_DEBUG)/src/Player.o \
	$(OBJDIR_DEBUG)/src/Util.o \
	$(OBJDIR_DEBUG)/src/World.o \
	$(OBJDIR_DEBUG)/src/chunk/Chunk.o \
	$(OBJDIR_DEBUG)/src/chunk/mesh/ChunkMesher.o \
	$(OBJDIR_DEBUG)/src/chunk/mesh/GreedyMesher.o \
	$(OBJDIR_DEBUG)/src/chunk/mesh/SimpleMesher.o \
	$(OBJDIR_DEBUG)/src/console/ArgumentParser.o \
	$(OBJDIR_DEBUG)/src/console/Command.o \
	$(OBJDIR_DEBUG)/src/console/command_test.o \
	$(OBJDIR_DEBUG)/src/console/Console.o \
	$(OBJDIR_DEBUG)/src/console/KeybindManager.o \
	$(OBJDIR_DEBUG)/src/event/Event.o \
	$(OBJDIR_DEBUG)/src/event/EventManager.o \
	$(OBJDIR_DEBUG)/src/event/type/Event_window_size_change.o \
	$(OBJDIR_DEBUG)/src/graphics/OpenGL/Shader.o \
	$(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexArray.o \
	$(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexBuffer.o \
	$(OBJDIR_DEBUG)/src/gui/GUI.o \
	$(OBJDIR_DEBUG)/src/physics/RaytraceHit.o \
	$(OBJDIR_DEBUG)/src/physics/Raytracer.o \
	$(OBJDIR_DEBUG)/src/position/BlockInChunk.o \
	$(OBJDIR_DEBUG)/src/position/BlockInWorld.o \
	$(OBJDIR_DEBUG)/src/position/ChunkInWorld.o \

OBJ_RELEASE = \
	$(OBJDIR_RELEASE)/glad.o \
	$(OBJDIR_RELEASE)/src/Block.o \
	$(OBJDIR_RELEASE)/src/Camera.o \
	$(OBJDIR_RELEASE)/src/Cube.o \
	$(OBJDIR_RELEASE)/src/Game.o \
	$(OBJDIR_RELEASE)/src/Gfx.o \
	$(OBJDIR_RELEASE)/src/main.o \
	$(OBJDIR_RELEASE)/src/Player.o \
	$(OBJDIR_RELEASE)/src/Util.o \
	$(OBJDIR_RELEASE)/src/World.o \
	$(OBJDIR_RELEASE)/src/chunk/Chunk.o \
	$(OBJDIR_RELEASE)/src/chunk/mesh/ChunkMesher.o \
	$(OBJDIR_RELEASE)/src/chunk/mesh/GreedyMesher.o \
	$(OBJDIR_RELEASE)/src/chunk/mesh/SimpleMesher.o \
	$(OBJDIR_RELEASE)/src/console/ArgumentParser.o \
	$(OBJDIR_RELEASE)/src/console/Command.o \
	$(OBJDIR_RELEASE)/src/console/command_test.o \
	$(OBJDIR_RELEASE)/src/console/Console.o \
	$(OBJDIR_RELEASE)/src/console/KeybindManager.o \
	$(OBJDIR_RELEASE)/src/event/Event.o \
	$(OBJDIR_RELEASE)/src/event/EventManager.o \
	$(OBJDIR_RELEASE)/src/event/type/Event_window_size_change.o \
	$(OBJDIR_RELEASE)/src/graphics/OpenGL/Shader.o \
	$(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexArray.o \
	$(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexBuffer.o \
	$(OBJDIR_RELEASE)/src/gui/GUI.o \
	$(OBJDIR_RELEASE)/src/physics/RaytraceHit.o \
	$(OBJDIR_RELEASE)/src/physics/Raytracer.o \
	$(OBJDIR_RELEASE)/src/position/BlockInChunk.o \
	$(OBJDIR_RELEASE)/src/position/BlockInWorld.o \
	$(OBJDIR_RELEASE)/src/position/ChunkInWorld.o \

all: debug release

clean: clean_debug clean_release

before_debug:
	test -d bin || mkdir -p bin
	test -d $(OBJDIR_DEBUG)/src || mkdir -p $(OBJDIR_DEBUG)/src
	test -d $(OBJDIR_DEBUG)/src/chunk || mkdir -p $(OBJDIR_DEBUG)/src/chunk
	test -d $(OBJDIR_DEBUG)/src/chunk/mesh || mkdir -p $(OBJDIR_DEBUG)/src/chunk/mesh
	test -d $(OBJDIR_DEBUG)/src/console || mkdir -p $(OBJDIR_DEBUG)/src/console
	test -d $(OBJDIR_DEBUG)/src/event || mkdir -p $(OBJDIR_DEBUG)/src/event
	test -d $(OBJDIR_DEBUG)/src/event/type || mkdir -p $(OBJDIR_DEBUG)/src/event/type
	test -d $(OBJDIR_DEBUG)/src/graphics || mkdir -p $(OBJDIR_DEBUG)/src/graphics
	test -d $(OBJDIR_DEBUG)/src/graphics/OpenGL || mkdir -p $(OBJDIR_DEBUG)/src/graphics/OpenGL
	test -d $(OBJDIR_DEBUG)/src/gui || mkdir -p $(OBJDIR_DEBUG)/src/gui
	test -d $(OBJDIR_DEBUG)/src/physics || mkdir -p $(OBJDIR_DEBUG)/src/physics
	test -d $(OBJDIR_DEBUG)/src/position || mkdir -p $(OBJDIR_DEBUG)/src/position

debug: before_debug out_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) -o $(OUT_DEBUG) $(OBJ_DEBUG) $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/glad.o: lib/glad/glad.c
	$(CC) $(CFLAGS_DEBUG) -c lib/glad/glad.c -o $(OBJDIR_DEBUG)/glad.o

$(OBJDIR_DEBUG)/src/Block.o: src/Block.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Block.cpp -o $(OBJDIR_DEBUG)/src/Block.o

$(OBJDIR_DEBUG)/src/Camera.o: src/Camera.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Camera.cpp -o $(OBJDIR_DEBUG)/src/Camera.o

$(OBJDIR_DEBUG)/src/Cube.o: src/Cube.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Cube.cpp -o $(OBJDIR_DEBUG)/src/Cube.o

$(OBJDIR_DEBUG)/src/Game.o: src/Game.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Game.cpp -o $(OBJDIR_DEBUG)/src/Game.o

$(OBJDIR_DEBUG)/src/Gfx.o: src/Gfx.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Gfx.cpp -o $(OBJDIR_DEBUG)/src/Gfx.o

$(OBJDIR_DEBUG)/src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/main.cpp -o $(OBJDIR_DEBUG)/src/main.o

$(OBJDIR_DEBUG)/src/Player.o: src/Player.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Player.cpp -o $(OBJDIR_DEBUG)/src/Player.o

$(OBJDIR_DEBUG)/src/Util.o: src/Util.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/Util.cpp -o $(OBJDIR_DEBUG)/src/Util.o

$(OBJDIR_DEBUG)/src/World.o: src/World.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/World.cpp -o $(OBJDIR_DEBUG)/src/World.o

$(OBJDIR_DEBUG)/src/chunk/Chunk.o: src/chunk/Chunk.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/chunk/Chunk.cpp -o $(OBJDIR_DEBUG)/src/chunk/Chunk.o

$(OBJDIR_DEBUG)/src/chunk/mesh/ChunkMesher.o: src/chunk/mesh/ChunkMesher.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/chunk/mesh/ChunkMesher.cpp -o $(OBJDIR_DEBUG)/src/chunk/mesh/ChunkMesher.o

$(OBJDIR_DEBUG)/src/chunk/mesh/GreedyMesher.o: src/chunk/mesh/GreedyMesher.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/chunk/mesh/GreedyMesher.cpp -o $(OBJDIR_DEBUG)/src/chunk/mesh/GreedyMesher.o

$(OBJDIR_DEBUG)/src/chunk/mesh/SimpleMesher.o: src/chunk/mesh/SimpleMesher.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/chunk/mesh/SimpleMesher.cpp -o $(OBJDIR_DEBUG)/src/chunk/mesh/SimpleMesher.o

$(OBJDIR_DEBUG)/src/console/ArgumentParser.o: src/console/ArgumentParser.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/console/ArgumentParser.cpp -o $(OBJDIR_DEBUG)/src/console/ArgumentParser.o

$(OBJDIR_DEBUG)/src/console/Command.o: src/console/Command.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/console/Command.cpp -o $(OBJDIR_DEBUG)/src/console/Command.o

$(OBJDIR_DEBUG)/src/console/command_test.o: src/console/command_test.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/console/command_test.cpp -o $(OBJDIR_DEBUG)/src/console/command_test.o

$(OBJDIR_DEBUG)/src/console/Console.o: src/console/Console.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/console/Console.cpp -o $(OBJDIR_DEBUG)/src/console/Console.o

$(OBJDIR_DEBUG)/src/console/KeybindManager.o: src/console/KeybindManager.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/console/KeybindManager.cpp -o $(OBJDIR_DEBUG)/src/console/KeybindManager.o

$(OBJDIR_DEBUG)/src/event/Event.o: src/event/Event.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/event/Event.cpp -o $(OBJDIR_DEBUG)/src/event/Event.o

$(OBJDIR_DEBUG)/src/event/EventManager.o: src/event/EventManager.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/event/EventManager.cpp -o $(OBJDIR_DEBUG)/src/event/EventManager.o

$(OBJDIR_DEBUG)/src/event/type/Event_window_size_change.o: src/event/type/Event_window_size_change.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/event/type/Event_window_size_change.cpp -o $(OBJDIR_DEBUG)/src/event/type/Event_window_size_change.o

$(OBJDIR_DEBUG)/src/graphics/OpenGL/Shader.o: src/graphics/OpenGL/Shader.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/graphics/OpenGL/Shader.cpp -o $(OBJDIR_DEBUG)/src/graphics/OpenGL/Shader.o

$(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexArray.o: src/graphics/OpenGL/VertexArray.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/graphics/OpenGL/VertexArray.cpp -o $(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexArray.o

$(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexBuffer.o: src/graphics/OpenGL/VertexBuffer.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/graphics/OpenGL/VertexBuffer.cpp -o $(OBJDIR_DEBUG)/src/graphics/OpenGL/VertexBuffer.o

$(OBJDIR_DEBUG)/src/gui/GUI.o: src/gui/GUI.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/gui/GUI.cpp -o $(OBJDIR_DEBUG)/src/gui/GUI.o

$(OBJDIR_DEBUG)/src/physics/RaytraceHit.o: src/physics/RaytraceHit.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/physics/RaytraceHit.cpp -o $(OBJDIR_DEBUG)/src/physics/RaytraceHit.o

$(OBJDIR_DEBUG)/src/physics/Raytracer.o: src/physics/Raytracer.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/physics/Raytracer.cpp -o $(OBJDIR_DEBUG)/src/physics/Raytracer.o

$(OBJDIR_DEBUG)/src/position/BlockInChunk.o: src/position/BlockInChunk.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/position/BlockInChunk.cpp -o $(OBJDIR_DEBUG)/src/position/BlockInChunk.o

$(OBJDIR_DEBUG)/src/position/BlockInWorld.o: src/position/BlockInWorld.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/position/BlockInWorld.cpp -o $(OBJDIR_DEBUG)/src/position/BlockInWorld.o

$(OBJDIR_DEBUG)/src/position/ChunkInWorld.o: src/position/ChunkInWorld.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -c src/position/ChunkInWorld.cpp -o $(OBJDIR_DEBUG)/src/position/ChunkInWorld.o

clean_debug:
	rm -f $(OUT_DEBUG)
	rm -rf $(OBJDIR_DEBUG)

before_release:
	test -d bin || mkdir -p bin
	test -d $(OBJDIR_RELEASE)/src || mkdir -p $(OBJDIR_RELEASE)/src
	test -d $(OBJDIR_RELEASE)/src/chunk || mkdir -p $(OBJDIR_RELEASE)/src/chunk
	test -d $(OBJDIR_RELEASE)/src/chunk/mesh || mkdir -p $(OBJDIR_RELEASE)/src/chunk/mesh
	test -d $(OBJDIR_RELEASE)/src/console || mkdir -p $(OBJDIR_RELEASE)/src/console
	test -d $(OBJDIR_RELEASE)/src/event || mkdir -p $(OBJDIR_RELEASE)/src/event
	test -d $(OBJDIR_RELEASE)/src/event/type || mkdir -p $(OBJDIR_RELEASE)/src/event/type
	test -d $(OBJDIR_RELEASE)/src/graphics || mkdir -p $(OBJDIR_RELEASE)/src/graphics
	test -d $(OBJDIR_RELEASE)/src/graphics/OpenGL || mkdir -p $(OBJDIR_RELEASE)/src/graphics/OpenGL
	test -d $(OBJDIR_RELEASE)/src/gui || mkdir -p $(OBJDIR_RELEASE)/src/gui
	test -d $(OBJDIR_RELEASE)/src/physics || mkdir -p $(OBJDIR_RELEASE)/src/physics
	test -d $(OBJDIR_RELEASE)/src/position || mkdir -p $(OBJDIR_RELEASE)/src/position

release: before_release out_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) -o $(OUT_RELEASE) $(OBJ_RELEASE) $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/glad.o: lib/glad/glad.c
	$(CC) $(CFLAGS_RELEASE) -c lib/glad/glad.c -o $(OBJDIR_RELEASE)/glad.o

$(OBJDIR_RELEASE)/src/Block.o: src/Block.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Block.cpp -o $(OBJDIR_RELEASE)/src/Block.o

$(OBJDIR_RELEASE)/src/Camera.o: src/Camera.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Camera.cpp -o $(OBJDIR_RELEASE)/src/Camera.o

$(OBJDIR_RELEASE)/src/Cube.o: src/Cube.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Cube.cpp -o $(OBJDIR_RELEASE)/src/Cube.o

$(OBJDIR_RELEASE)/src/Game.o: src/Game.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Game.cpp -o $(OBJDIR_RELEASE)/src/Game.o

$(OBJDIR_RELEASE)/src/Gfx.o: src/Gfx.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Gfx.cpp -o $(OBJDIR_RELEASE)/src/Gfx.o

$(OBJDIR_RELEASE)/src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/main.cpp -o $(OBJDIR_RELEASE)/src/main.o

$(OBJDIR_RELEASE)/src/Player.o: src/Player.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Player.cpp -o $(OBJDIR_RELEASE)/src/Player.o

$(OBJDIR_RELEASE)/src/Util.o: src/Util.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/Util.cpp -o $(OBJDIR_RELEASE)/src/Util.o

$(OBJDIR_RELEASE)/src/World.o: src/World.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/World.cpp -o $(OBJDIR_RELEASE)/src/World.o

$(OBJDIR_RELEASE)/src/chunk/Chunk.o: src/chunk/Chunk.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/chunk/Chunk.cpp -o $(OBJDIR_RELEASE)/src/chunk/Chunk.o

$(OBJDIR_RELEASE)/src/chunk/mesh/ChunkMesher.o: src/chunk/mesh/ChunkMesher.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/chunk/mesh/ChunkMesher.cpp -o $(OBJDIR_RELEASE)/src/chunk/mesh/ChunkMesher.o

$(OBJDIR_RELEASE)/src/chunk/mesh/GreedyMesher.o: src/chunk/mesh/GreedyMesher.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/chunk/mesh/GreedyMesher.cpp -o $(OBJDIR_RELEASE)/src/chunk/mesh/GreedyMesher.o

$(OBJDIR_RELEASE)/src/chunk/mesh/SimpleMesher.o: src/chunk/mesh/SimpleMesher.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/chunk/mesh/SimpleMesher.cpp -o $(OBJDIR_RELEASE)/src/chunk/mesh/SimpleMesher.o

$(OBJDIR_RELEASE)/src/console/ArgumentParser.o: src/console/ArgumentParser.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/console/ArgumentParser.cpp -o $(OBJDIR_RELEASE)/src/console/ArgumentParser.o

$(OBJDIR_RELEASE)/src/console/Command.o: src/console/Command.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/console/Command.cpp -o $(OBJDIR_RELEASE)/src/console/Command.o

$(OBJDIR_RELEASE)/src/console/command_test.o: src/console/command_test.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/console/command_test.cpp -o $(OBJDIR_RELEASE)/src/console/command_test.o

$(OBJDIR_RELEASE)/src/console/Console.o: src/console/Console.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/console/Console.cpp -o $(OBJDIR_RELEASE)/src/console/Console.o

$(OBJDIR_RELEASE)/src/console/KeybindManager.o: src/console/KeybindManager.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/console/KeybindManager.cpp -o $(OBJDIR_RELEASE)/src/console/KeybindManager.o

$(OBJDIR_RELEASE)/src/event/Event.o: src/event/Event.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/event/Event.cpp -o $(OBJDIR_RELEASE)/src/event/Event.o

$(OBJDIR_RELEASE)/src/event/EventManager.o: src/event/EventManager.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/event/EventManager.cpp -o $(OBJDIR_RELEASE)/src/event/EventManager.o

$(OBJDIR_RELEASE)/src/event/type/Event_window_size_change.o: src/event/type/Event_window_size_change.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/event/type/Event_window_size_change.cpp -o $(OBJDIR_RELEASE)/src/event/type/Event_window_size_change.o

$(OBJDIR_RELEASE)/src/graphics/OpenGL/Shader.o: src/graphics/OpenGL/Shader.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/graphics/OpenGL/Shader.cpp -o $(OBJDIR_RELEASE)/src/graphics/OpenGL/Shader.o

$(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexArray.o: src/graphics/OpenGL/VertexArray.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/graphics/OpenGL/VertexArray.cpp -o $(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexArray.o

$(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexBuffer.o: src/graphics/OpenGL/VertexBuffer.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/graphics/OpenGL/VertexBuffer.cpp -o $(OBJDIR_RELEASE)/src/graphics/OpenGL/VertexBuffer.o

$(OBJDIR_RELEASE)/src/gui/GUI.o: src/gui/GUI.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/gui/GUI.cpp -o $(OBJDIR_RELEASE)/src/gui/GUI.o

$(OBJDIR_RELEASE)/src/physics/RaytraceHit.o: src/physics/RaytraceHit.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/physics/RaytraceHit.cpp -o $(OBJDIR_RELEASE)/src/physics/RaytraceHit.o

$(OBJDIR_RELEASE)/src/physics/Raytracer.o: src/physics/Raytracer.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/physics/Raytracer.cpp -o $(OBJDIR_RELEASE)/src/physics/Raytracer.o

$(OBJDIR_RELEASE)/src/position/BlockInChunk.o: src/position/BlockInChunk.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/position/BlockInChunk.cpp -o $(OBJDIR_RELEASE)/src/position/BlockInChunk.o

$(OBJDIR_RELEASE)/src/position/BlockInWorld.o: src/position/BlockInWorld.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/position/BlockInWorld.cpp -o $(OBJDIR_RELEASE)/src/position/BlockInWorld.o

$(OBJDIR_RELEASE)/src/position/ChunkInWorld.o: src/position/ChunkInWorld.cpp
	$(CXX) $(CXXFLAGS_RELEASE) -c src/position/ChunkInWorld.cpp -o $(OBJDIR_RELEASE)/src/position/ChunkInWorld.o

clean_release:
	rm -f $(OUT_RELEASE)
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release
