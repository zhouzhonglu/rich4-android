# 安卓顶层 CMake：SDL3 + FreeType + rich4（编成 libmain.so 供 SDLActivity 加载）。
# 对照 desktop 版 CMakeLists.txt：rich4_core 全部源码 + main.cpp → 单个 SHARED 库。
cmake_minimum_required(VERSION 3.22)
project(rich4_android LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# [PORT 安卓] 16KB 页对齐：安卓 15+ 设备按 16KB 页加载 .so，LOAD 段仍按 4KB 对齐
# 会在 loadLibrary 时直接崩溃（表现为「装完闪退」）。NDK r26 默认 4KB，需显式指定。
add_link_options(-Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384)

# ---- SDL3（jni/SDL 软链到 third_party/SDL3-src）----
# 安卓走默认 SHARED：SDLActivity 通过 System.loadLibrary("SDL3") + ("main") 装载。
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(SDL)

# ---- FreeType（文本渲染，纯 C 无外部依赖）----
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/freetype/CMakeLists.txt")
  add_subdirectory(freetype)
  set(RICH4_FREETYPE_TARGET freetype)
else()
  include(FetchContent)
  FetchContent_Declare(freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG VER-2-14-3
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(freetype)
  set(RICH4_FREETYPE_TARGET freetype)
endif()

add_subdirectory(src)
