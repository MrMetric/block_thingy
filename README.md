# block thingy [![Coverity Scan Build Status](https://scan.coverity.com/projects/7790/badge.svg)](https://scan.coverity.com/projects/mrmetric-block_thingy)
I mek GAme

## Features
 * can render tens of blocks at once
 * [will not delete your home folder](https://github.com/valvesoftware/steam-for-linux/issues/3671)
 * free for at least 1 more day
 * has no bugs (that I know of)
 * never gonna give you up
 * Y2K-compliant
 * has a license (GPL v3)

## How to get it
### Cloning
As this repository has submodules, you need to clone it recursively:

    $ git clone --recursive https://gitlab.com/MrMetric/block_thingy.git

If you have already cloned it and did not do that, then you need to init and update submodules:

    $ cd block_thingy
    $ git submodule update --init --recursive

If you are using Windows, you must run `fix_symlinks.bat` before building or running block_thingy. This is because Git incorrectly translates each symbolic link to be just a text file with the link path as the content.

### Dependencies
thing | included?
----- | ---------
[FreeType 2](https://www.freetype.org/) | ❌
[glad](https://github.com/Dav1dde/glad) | ✔ (direct)
[GLFW 3](https://github.com/glfw/glfw) | ❌
[GLM](https://github.com/g-truc/glm) | ✔ (submodule)
libpng | ❌
[moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue) | ✔ (direct)
[msgpack-c](https://github.com/msgpack/msgpack-c/) | ❌
[rhea 0.3](https://github.com/Nocte-/rhea/tree/0.3) | ✔ (submodule)
[strict variant](https://github.com/cbeck88/strict-variant) | ✔ (submodule)
[webgl-noise](https://github.com/stegu/webgl-noise) | ✔ (submodule)

### Building (using CMake)

Compiler version requirements:

 * GCC: >= 6
 * Clang: >= 3.6

Make a directory to build in, and run `cmake <repo root path>`, along with whatever options you want CMake to use.
For example, in the repo root:

```shell
$ mkdir build
$ cmake ..
$ make
```

The build type options are:

 * Debug
 * Release
 * RelWithDebInfo (default)
 * MinSizeRel

To specify Release (for example):

```shell
$ cmake .. -DCMAKE_BUILD_TYPE=release
```

Note: the build types are not case-sensitive.

To use a different compiler:

```shell
$ cmake .. -DCMAKE_C_COMPILER="/usr/local/bin/clang" -DCMAKE_CXX_COMPILER="/usr/local/bin/clang++"
```

### Running

To run, `block_thingy` needs to know where the game files are. It defaults to `.`, which is incorrect here—the folder is `<repo root>/bin`. Following from the above example, do this:

```shell
./block_thingy ../bin
```

## License

GPLv3, with one exception: creating non-free plugins is permitted.

## TODO
 * add a more useful error message for when game files are not found
 * add frustum culling
 * add other meshing methods [[1]](http://0fps.net/2012/07/07/meshing-minecraft-part-2/) [[2]](https://blackflux.wordpress.com/2014/02/23/meshing-in-voxel-engines-part-1/)
 * add texturing
 * allow using image formats other than PNG
 * improve build instructions
 * make physics framerate-independent
 * move the commands from `Game` to somewhere else
 * separate chunk data / rendering (this decouples meshing from chunking, and has the side benefit of allowing mesh dimensions to not match chunks)
 * use events more (look at: keypress, mousemove, …)
 * continue running when adding inotify watches fails
 * watch files on Windows®
 * other stuff that I am too lazy to say here

## Note
This repository exists in multiple places:

 * [GitLab](https://gitlab.com/MrMetric/block_thingy)
 * [GitHub](https://github.com/MrMetric/block_thingy)

GitLab is the preferred location for posting issues and submitting merge requests (AKA pull requests)
