ADD_EXTERNAL_DEPS()
# find_package(glfw3 3.4 REQUIRED)
# find_package(OpenGL REQUIRED)

add_library(VisualizerLib SHARED
    datasets/YarrBinaryFile.cpp
    datasets/AllDataLoaders.cpp
    util/include/json.hpp
    util/include/util.hpp
    util/include/logging.h
    util/logging.cpp
)

target_include_directories(VisualizerLib PUBLIC datasets/include)
target_include_directories(VisualizerLib PUBLIC util/include)

add_executable(test 
    core/test.cpp
)
target_link_libraries(test VisualizerLib pthread rt glfw glad imgui)
set_target_properties(test 
    PROPERTIES 
    LINKER_LANGUAGE CXX
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_INSTALL_AREA}/bin"
)

add_executable(datatest
    core/datareader_test.cpp
)
target_link_libraries(datatest VisualizerLib pthread)
set_target_properties(datatest
    PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_INSTALL_AREA}/bin"
)

# message("Saving bin files to ${TARGET_INSTALL_AREA}")
