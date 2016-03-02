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

To run, `block_thingy` needs to be executed from the `bin` directory. So, following from the example above:

```shell
cd ../bin
../build/block_thingy
```

## TODO
 * improve build instructions
 * fix dependency on running in the `bin` directory
 * allow saving screenshots to a format other than PNG (for when the libpng dependency is not enabled)
 * make physics framerate-independent
 * split Game into separate thingies
 * add other meshing methods [[1]](http://0fps.net/2012/07/07/meshing-minecraft-part-2/) [[2]](https://blackflux.wordpress.com/2014/02/23/meshing-in-voxel-engines-part-1/)
 * add texturing
 * add frustum culling
 * other stuff that I am too lazy to say here
