# block thingy
I mek GAme

## Features
 * can render tens of blocks at once
 * [will not delete your home folder](https://github.com/valvesoftware/steam-for-linux/issues/3671)
 * free for at least 1 more day
 * has no bugs (that I know of)
 * never gonna give you up
 * Y2K-compliant
 * has a license (GPL v3)

## hoa 2 get dis shizzle
### Cloning
As this repository contains a submodule, you will need to clone it recursively:

    $ git clone --recursive https://github.com/MrMetric/block_thingy.git

If you have already cloned it and did not do that, then you need to init and update submodules:

    $ cd block_thingy
    $ git submodule update --init --recursive

### Dependencies
 * glm: included as a submodule
 * glad: included in the repository
 * GLFW 3.1.2: installing it with your distro's package manager _might_ work. I recommend building it from source instead ([glfw-3.1.2.zip](https://github.com/glfw/glfw/releases/download/3.1.2/glfw-3.1.2.zip)).
 * libpng: optional, used for saving screenshots. To enable it, `USE_LIBPNG` must be defined when compiling. Note that compiling without this disables screenshots entirely.
 * [POCO](https://github.com/pocoproject/poco)

### Building
 * versions of GCC older than 5.3.0 **can not** compile this
 * clang 3.7 and 3.9 both work fine; older versions are untested, although 3.5 is reported to work

If you are unfamiliar with CMake: Make a directory to build in, and run `cmake <repo root path>`, along with whatever options you want CMake to use.
For example, in the repo root:

```shell
mkdir build
cmake ..
make
```

Or, to specify a different compiler:

```shell
$ cmake .. -DCMAKE_C_COMPILER="/usr/local/bin/clang" -DCMAKE_CXX_COMPILER="/usr/local/bin/clang++"
```

To run, `block_thingy` needs to know where the game files are. It defaults to `.`, which is incorrect here—the folder is `<repo root>/bin`. Following from the example above, simply do this:

```shell
./block_thingy ../bin
```

## TODO
 * add a more useful error message for when game files are not found
 * add frustum culling
 * add other meshing methods [[1]](http://0fps.net/2012/07/07/meshing-minecraft-part-2/) [[2]](https://blackflux.wordpress.com/2014/02/23/meshing-in-voxel-engines-part-1/)
 * add POCO to CMake file
 * add texturing
 * allow saving screenshots to a format other than PNG (for when the libpng dependency is not enabled)
 * improve build instructions
 * make physics framerate-independent
 * move the commands from `Game` and `command_test` to somewhere else
 * separate chunk data / rendering (this decouples meshing from chunking, and has the side benefit of allowing mesh dimensions to not match chunks)
 * use events more (look at: keypress, mousemove, …)
 * other stuff that I am too lazy to say here
