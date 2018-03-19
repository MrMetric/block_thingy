# block thingy [![Coverity Scan Build Status](https://scan.coverity.com/projects/7790/badge.svg)](https://scan.coverity.com/projects/mrmetric-block_thingy)
I mek GAme

## What is this?

block_thingy is a Free and Open-Source voxel game engine. It is very much a work-in-progress, so don't expect to see lots of fancy features (yet). It is not a clone of Minecraft, although making a Minecraft clone with it will be possible once the required features are implemented.

## Some features
 * uses cubic chunks and has no arbitrary low height limit
 * has 3-channel (RGB) flood fill lighting
 * FOSS (license is GPLv3)
 * [will not delete your home folder](https://github.com/valvesoftware/steam-for-linux/issues/3671)
 * Y2K-compliant

## How to clone
As this repository has submodules, you need to clone it recursively:

```shell
$ git clone --recursive https://gitlab.com/MrMetric/block_thingy.git
```

If you have already cloned it and did not do that, then you need to init and update submodules:

```shell
$ cd block_thingy
$ git submodule update --init --recursive
```

If you are using Windows, you must run `fix_symlinks.bat` before building or running block_thingy. This is because Git incorrectly translates each symbolic link to be just a text file with the link path as the content. If you are using Windows 10, you must enable [developer mode](https://docs.microsoft.com/en-us/windows/uwp/get-started/enable-your-device-for-development) for `fix_symlinks.bat` to work. Refer to this official Windows blog post for more information: [Symlinks in Windows 10!](https://blogs.windows.com/buildingapps/2016/12/02/symlinks-windows-10/).

## Dependencies
thing | included? | license
----- | --------- | -------
[FreeType 2](https://www.freetype.org/) | ❌ | [GPLv2 \| FTL (compatible with GPLv3)](https://www.freetype.org/license.html)
[glad](https://github.com/Dav1dde/glad) | ✔ (direct) | [MIT](https://github.com/Dav1dde/glad/blob/master/LICENSE)
[GLFW 3](https://github.com/glfw/glfw) | ❌ | [zlib](https://github.com/glfw/glfw/blob/master/LICENSE.md)
[GLM](https://github.com/g-truc/glm) | ✔ (submodule) | MIT
libpng | ❌ | libpng
[moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue) | ✔ (direct) | [simplified BSD \| Boost](https://github.com/cameron314/concurrentqueue/blob/master/LICENSE.md)
[msgpack-c](https://github.com/msgpack/msgpack-c/) | ❌ | [Boost](https://github.com/msgpack/msgpack-c/blob/master/LICENSE_1_0.txt)
[rhea 0.3](https://github.com/Nocte-/rhea/tree/0.3) | ✔ (submodule) | [MIT](https://github.com/Nocte-/rhea/blob/0.3/LICENSE.md)
[strict variant](https://github.com/cbeck88/strict-variant) | ✔ (submodule) | [Boost](https://github.com/cbeck88/strict-variant/blob/master/LICENSE)
[webgl-noise](https://github.com/stegu/webgl-noise) | ✔ (submodule) | [MIT](https://github.com/stegu/webgl-noise/blob/master/LICENSE)

## GNU/Linux

### Building (using CMake)

The only compilers that I guarantee will work are:

 * GCC == 7.3.1
 * Clang == 5.0.1

Other versions of these compilers might work, but I do not guarantee that they do. I also do not guarantee that any other compilers work, although I do try to stick to standard C++ and use `#ifdef`s around non-standard features.

Make a directory to build in, and run `cmake <repo root path>`, along with whatever options you want CMake to use.
For example, in the repo root:

```shell
$ mkdir build
$ cd build
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

To use a different compiler, there are two methods:

```shell
$ CC=/usr/local/bin/clang CXX=/usr/local/bin/clang++ cmake .."
```

```shell
$ cmake .. -DCMAKE_C_COMPILER="/usr/local/bin/clang" -DCMAKE_CXX_COMPILER="/usr/local/bin/clang++"
```

### Running

To run, `block_thingy` needs to know where the game files are. It defaults to `.`, which is incorrect here—the folder is `<repo root>/bin`. Following from the above example, do this:

```shell
$ ./block_thingy ../bin
```

## Windows

### Building

I have tested building with Visual Studio Community 2017 version 15.6.2. I do not guarantee that any other version will work, but future 2017 versions should be fine.

Install [vcpkg](https://github.com/Microsoft/vcpkg) according to its instructions. Note that vcpkg's defaults to installing 32-bit libraries. Install the dependencies like this:

```
vcpkg install --triplet x64-windows freetype glfw3 libpng msgpack
```

If you are building for 32-bit, omit `--triplet x64-windows`.

Next, open `projects/vc2017/block_thingy.sln` with Visual Studio. Choose your desired configuration and architecture and build it (Build → Build Solution).

### Running

Just run block_thingy.exe. It should automatically detect where the bin directory is. If it does not, specify the path as the first argument (refer to the GNU/Linux instructions above).

## License

GPLv3, with one exception: creating non-free plugins is permitted. Opinions as to whether or not the GPL allows this vary, so I am explicitly allowing it here. If the maintainers of any GPL libraries I use object to this, let me know and I will stop using your library.

## TODO
 * add a more useful error message for when game files are not found
 * make block accesses faster (this will make meshing and light propagation faster)
 * add other meshing methods [[1]](http://0fps.net/2012/07/07/meshing-minecraft-part-2/) [[2]](https://blackflux.wordpress.com/2014/02/23/meshing-in-voxel-engines-part-1/)
 * allow using image formats other than PNG (JPEG, WebP, BMP, …)
 * make physics framerate-independent
 * move the commands from `Game` to somewhere else
 * separate chunk data / rendering (this decouples meshing from chunking, and has the side benefit of allowing mesh dimensions to not match chunks)
 * use an event queue and use events more (look at: keypress, mousemove, …)
 * continue running when adding inotify watches fails
 * watch files on Windows®
 * add more things to the feature list
 * other stuff that I am too lazy to say here

## Note
This repository exists in multiple places:

 * [GitLab](https://gitlab.com/MrMetric/block_thingy)
 * [GitHub](https://github.com/MrMetric/block_thingy)

GitLab is the preferred location for posting issues and submitting merge requests (AKA pull requests)
