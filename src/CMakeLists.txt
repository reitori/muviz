ADD_EXTERNAL_DEPS()

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
target_link_libraries(test VisualizerLib pthread rt)
set_target_properties(test PROPERTIES LINKER_LANGUAGE CXX)
