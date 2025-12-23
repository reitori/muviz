cmake_minimum_required(VERSION 3.8)

project(Visualizer VERSION 1.0.0)
set(RUNTIME_INSTALL_AREA "${PROJECT_SOURCE_DIR}/bin")

#Compiler flags for build type
if(CMAKE_BUILD_TYPE MATCHES ASan)
    add_compile_options(-g -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
    message("Enabling address sanitizer")
elseif(CMAKE_BUILD_TYPE MATCHES LSan)
    add_compile_options( -g -fsanitize=leak)
    add_link_options(-fsanitize=leak)
    message("Enabling leak sanitizer")
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g)
    set(CMAKE_C_FLAGS_DEBUG "-g" CACHE STRING "C Flags for Debug" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "C++ Flags for Debug" FORCE)
    message("Using debug build type")
else()
    add_compile_options(-O2)
    message("Using optimized build type")
endif()

#Ensure C++17 is in use
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Ensure that GLVND Libraries are included over legacy
cmake_policy (SET CMP0072 NEW)

#Required packages
find_package(Git REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
include(cmake/CMakeLists.txt.external)

#Dependencies Subdir
add_subdirectory(deps/glad/)
add_subdirectory(deps/imgui/)
add_subdirectory(deps/stb/)

#Application subdir
add_subdirectory(src)


#Copy assets to binary output directory
#Visualizer needs assets folder within same relative path
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_BINARY_DIR "${CMAKE_BINARY_DIR}/assets")
file(COPY ${ASSETS_SOURCE_DIR} DESTINATION "${CMAKE_BINARY_DIR}/assets")

