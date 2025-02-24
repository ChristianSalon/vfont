cmake_minimum_required(VERSION 3.25)

# freetype initialization
set(FT_DISABLE_ZLIB ON CACHE BOOL "a" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "a" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "a" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "a" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "a" FORCE)

add_subdirectory(freetype)

# glm initialization
add_subdirectory(glm)

# CDT initialization
add_subdirectory(CDT/CDT)

# harfbuzz initialization
add_subdirectory(harfbuzz)
