# VFONT

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

**vfont** is a C++ library for working with vector fonts. It supports Unicode text, performs basic layout operations, and generates renderable geometry for glyphs. Rendering is handled via a built-in Vulkan backend. The renderer is modular and base classes can be extended to implement various rendering backends. Documentation available at https://christiansalon.github.io/vfont/.

Currently implemented rendering methods:
- Triangle Meshes
- Tessellation Shaders
- Signed Distance Fields (SDF)
- Winding Number Algorithm

![Demo app](figures/vfont-demo-app.png)

## Features

- UTF-8, UTF-16 or UTF-32 encoded text
- Font loading and glyph rasterization using FreeType
- Text shaping using HarfBuzz
- Basic text layout
- High-performance Vulkan backend

### Prerequisites

- CMake 3.25+
- A C++20 compatible compiler
- Vulkan SDK

## Building

Supported windowing systems are `Win32`, `X11` and `Wayland`. The default windowing system on Linux is X11. If you want to build documentation, use `-DBUILD_DOCS=ON`. If you want to build examples, use `-DBUILD_EXAMPLES=ON`.

Example of building on Linux with make:

```
git clone --recurse-submodules https://github.com/ChristianSalon/vfont.git
mkdir build && cd build
cmake ..
make
```

### Win32

To explicitly build for Win32, use `-DPLATFORM=USE_WIN32`.

**Windows SDK** is required.

### X11

To explicitly build for X11 using Xlib, use `-DPLATFORM=USE_X11`.

Required library is **xlib**.

### Wayland

To explicitly build for Wayland, use `-DPLATFORM=USE_WAYLAND`.

Libraries required are **wayland**, **wayland-protocols** and **xkbcommon**.

## Run Demo Scene

Always run examples from the directory where the executables are located. This is necessary for correct shader and font path resolution.

```
./demo [-h] [-c <perspective/orthographic>] [-a <cdt/ts/wn/sdf>] [-t] [-m]
```

### Options
- `-h`: Show help message
- `-c`: Select the type of camera used
    -   perspective - Perspetcive camera
    -   orthographic - Orthographic camera
- `-a`: Select the rendering algorithm
    -   cdt - Constrained delaunay triangulation on the cpu
    -   ts - Outer triangles processed by tessellation shaders, inner triangulated on the cpu
    -   wn - Winding number calculated in fragment shader
    -   sdf - Signed distance fields
- `-t`: Measure the gpu draw time
- `-m`: Use multisampling antialiasing

### Controls

- **Move:** Hold **Left Mouse Button** and move the mouse.
- **Rotate:** Hold **Right Mouse Button** and move the mouse.
- **Zoom:** Use the **Scroll Wheel**.

## Used Libraries

- [Freetype](https://github.com/freetype/freetype)
- [HarfBuzz](https://github.com/harfbuzz/harfbuzz)
- [glm](https://github.com/icaven/glm)
- [CDT](https://github.com/artem-ogre/CDT)
