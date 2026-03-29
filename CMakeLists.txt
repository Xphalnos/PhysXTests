cmake_minimum_required(VERSION 3.25)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(PhysXTests)

if (WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(-DPX_SIMD_DISABLED) # FIXME: clang-cl creates errors without that
endif()

add_compile_options(-Wno-inconsistent-missing-override)

add_compile_options(-msse4.2)
add_compile_options(-flto=full)

include_directories(PhysX/include)
link_directories(PhysX/lib)
include_directories(raylib/src)
add_subdirectory(raylib)

add_executable(PhysXTests Main.cpp)

# Link libraries
target_link_libraries(PhysXTests raylib PhysX_static_64 PhysXCommon_static_64 PhysXCooking_static_64 PhysXExtensions_static_64 PhysXFoundation_static_64)
