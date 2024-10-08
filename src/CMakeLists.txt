ADD_EXTERNAL_DEPS()
# find_package(glfw3 3.4 REQUIRED)
# find_package(OpenGL REQUIRED)

add_library(VisualizerLib SHARED
    datasets/YarrBinary.cpp
    util/include/json.hpp
    util/include/util.hpp
    util/include/logging.h
    util/logging.cpp
)

target_include_directories(VisualizerLib PUBLIC datasets/include)
target_include_directories(VisualizerLib PUBLIC util/include)

add_executable(test 
    core/test.cpp
    datasets/YarrBinary.cpp
)
target_link_libraries(test VisualizerLib pthread rt glfw glad)
set_target_properties(test PROPERTIES LINKER_LANGUAGE CXX)
