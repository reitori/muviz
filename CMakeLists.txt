cmake_minimum_required(VERSION 3.8)
find_package(Git REQUIRED)

project(Visualizer VERSION 1.0.0)
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(TARGET_INSTALL_AREA ${PROJECT_SOURCE_DIR})

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




# ExternalProject_Add (
#       YARR
#       GIT_REPOSITORY https://gitlab.cern.ch/YARR/YARR.git
#       #GIT_TAG 9b1d3da74dfc3ca1687c8ab581f889351333395a
#       GIT_TAG devel
#       INSTALL_COMMAND ""
#       PREFIX "${CMAKE_CURRENT_BINARY_DIR}/YARR"
#       #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} )
#       CMAKE_ARGS
#             -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
#             -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
#       )

# ExternalProject_Add (
#       EUDAQ
#       GIT_REPOSITORY https://github.com/arummler/eudaq.git
#       GIT_TAG master-versioning
#       PREFIX "${CMAKE_CURRENT_BINARY_DIR}/EUDAQ"
#       #CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} )
#       CMAKE_ARGS
#             -DEUDAQ_BUILD_GUI=OFF
#             -DEUDAQ_BUILD_EXECUTABLE=OFF
#             -DUSER_ADENIUM_BUILD=OFF
#             -DUSER_EUDET_BUILD=OFF
#             -DUSER_EXAMPLE_BUILD=OFF
#             -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
#             -DCMAKE_CXX_STANDARD_REQUIRED=${CMAKE_CXX_STANDARD_REQUIRED}
#       )

include(cmake/CMakeLists.txt.external)
add_subdirectory(src)
cmake_policy (SET CMP0072 OLD)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# install(TARGETS test DESTINATION ${PROJECT_BINARY_DIR}/bin)

# find_package(glfw REQUIRED)
# target_include_directories(VisualizerLib PUBLIC include)
# install(DIRECTORY ${PROJECT_BINARY_DIR}/bin DESTINATION ${TARGET_INSTALL_AREA}/bin  FILES_MATCHING PATTERN "*" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#     WORLD_EXECUTE WORLD_READ GROUP_EXECUTE GROUP_READ  )
# install(DIRECTORY ${PROJECT_BINARY_DIR}/lib DESTINATION ${TARGET_INSTALL_AREA}  FILES_MATCHING PATTERN "b" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#     WORLD_EXECUTE WORLD_READ GROUP_EXECUTE GROUP_READ  )