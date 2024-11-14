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
    OpenGL/Shader.cpp
    datasets/YarrBinary.cpp
)
target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test VisualizerLib pthread rt glfw glad imgui glm)
set_target_properties(test 
    PROPERTIES 
    LINKER_LANGUAGE CXX
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_INSTALL_AREA}/bin"
)

# message("Saving bin files to ${TARGET_INSTALL_AREA}")