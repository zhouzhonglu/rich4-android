# rich4 安卓库：全部源码（含 main.cpp）编成 libmain.so，由 SDLActivity 加载。
# 源码实际位于仓库根 src/（桌面版 CMakeLists 的 rich4_core + rich4 exe 合并）。
cmake_minimum_required(VERSION 3.22)
project(rich4_android_app)

set(RICH4_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../..")

if(NOT TARGET SDL3::SDL3)
    find_package(SDL3 CONFIG)
endif()
if(NOT TARGET SDL3::SDL3)
    message(FATAL_ERROR "Cannot find SDL3 — jni/CMakeLists.txt 需先 add_subdirectory(SDL)")
endif()

file(GLOB_RECURSE RICH4_SOURCES CONFIGURE_DEPENDS "${RICH4_ROOT}/src/*.cpp")

add_library(main SHARED ${RICH4_SOURCES})

target_include_directories(main PRIVATE
    "${RICH4_ROOT}/include"
    "${RICH4_ROOT}/third_party/stb")

target_link_libraries(main PRIVATE SDL3::SDL3 ${RICH4_FREETYPE_TARGET})

# Android 需要 log 库（RICH4_LOG* 写 logcat）
find_library(ANDROID_LOG_LIB log)
if(ANDROID_LOG_LIB)
    target_link_libraries(main PRIVATE ${ANDROID_LOG_LIB})
endif()

target_compile_options(main PRIVATE -Wall -Wextra)

target_link_options(main PRIVATE -u ANativeActivity_onCreate)
