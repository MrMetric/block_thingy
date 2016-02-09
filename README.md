# block thingy
I mek GAme

## Features
 * can render tens of blocks at once
 * [will not delete your home folder](https://github.com/valvesoftware/steam-for-linux/issues/3671)
 * free for at least 1 more day
 * has no bugs (that I know of)
 * never gonna give you up
 * Y2K-compliant

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

### Building
 * versions of GCC older than 5.3.0 **can not** compile this
 * clang 3.5 or newer works fine; older versions are untested
 * the linker options in the makefile are specific to my system, so you will likely need to tweak them

## TODO
 * use something nicer than the current makefile (portability is a concern)
 * improve build instructions
 * allow saving screenshots to a format other than PNG (for when the libpng dependency is not enabled)
 * make physics framerate-independent
 * split Game into separate thingies
 * make an actual working greedy mesher
 * add other meshing methods [[1]](http://0fps.net/2012/07/07/meshing-minecraft-part-2/) [[2]](https://blackflux.wordpress.com/2014/02/23/meshing-in-voxel-engines-part-1/)
 * add texturing
 * add frustum culling
 * other stuff that I am too lazy to say here
