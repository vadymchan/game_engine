cmake_minimum_required(VERSION 3.26)
project(tools NONE)

if(DEFINED TRACY_VERSION)
    set(TRACY_VERSION ${TRACY_VERSION} CACHE STRING "Tracy version" FORCE)
endif()

if(BUILD_ASSET_TOOLS)
    add_subdirectory(asset_tools)
endif()

if(BUILD_PROFILING_TOOLS)
    add_subdirectory(profiling_tools)
endif()