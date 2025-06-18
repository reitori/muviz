cmake_minimum_required(VERSION 3.8)
find_package(Git REQUIRED)

project(Visualizer VERSION 1.0.0)
set(RUNTIME_INSTALL_AREA "${PROJECT_SOURCE_DIR}/bin")

if(CMAKE_BUILD_TYPE MATCHES Asan)
    add_compile_options(-g -fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
    message("Enabling address sanitizer")
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g)
    message("Using debug build type")
else()
    add_compile_options(-O2)
    message("Using optimized build type")
endif()

# option(BUILD_TESTS "Make tests" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Dependencies Subdir
add_subdirectory(deps/glad/)
add_subdirectory(deps/imgui/)
add_subdirectory(deps/stb/)


#Copy assets to binary output directory
#Visualizer needs assets folder within same relative path
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_BINARY_DIR "${CMAKE_BINARY_DIR}/assets")
file(COPY ${ASSETS_SOURCE_DIR} DESTINATION "${CMAKE_BINARY_DIR}/assets")

include(cmake/CMakeLists.txt.external)
add_subdirectory(src)
cmake_policy (SET CMP0072 OLD)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

