# Keyboard Input & Output (KIO)

## Build and Run

**Vulkan** is required in this project. Supported windowing systems are Win32, X11 and Wayland. The default windowing system on Linux is X11.

Example for building and running on Linux with make. If you want to build documentation, use `cmake .. -DBUILD_DOC=ON`.

```
mkdir build && cd build
cmake ..
make
cd src
./kio
```

### Win32

To explicitly build for Win32, instead of `cmake ..` use `cmake .. -DUSE_WIN32`.

**Windows SDK** is required.

### X11

To explicitly build for X11 using Xlib, instead of `cmake ..` use `cmake .. -DUSE_X11`.

Required library is **xlib**.

### Wayland

To explicitly build for Wayland, instead of `cmake ..` use `cmake .. -DUSE_WAYLAND`.

Libraries required are **wayland**, **wayland-protocols** and **xkbcommon**.

## Used libraries

- [Freetype](https://www.freetype.org)
- [CDT](https://github.com/artem-ogre/CDT)
