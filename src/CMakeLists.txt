ADD_EXTERNAL_DEPS()
# find_package(glfw3 3.4 REQUIRED)
# find_package(OpenGL REQUIRED)

add_library(VisualizerLib SHARED
    datasets/YarrBinaryFile.cpp
    datasets/SocketReceiver.cpp
    datasets/SocketSubscriber.cpp
    datasets/AllDataLoaders.cpp
    util/include/json.hpp
    util/include/util.hpp
    util/include/logging.h
    util/include/CircularBuffer.h
    util/logging.cpp
    util/cli.cpp
    util/mathtools.cpp
)

target_link_libraries(VisualizerLib PUBLIC pthread rt glfw glad imgui glm stb libzmq)
target_include_directories(VisualizerLib PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(VisualizerLib PUBLIC datasets/include)
target_include_directories(VisualizerLib PUBLIC util/include)

add_executable(test 
    core/test.cpp
    OpenGL/Shader.cpp
)

target_include_directories(test PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(test VisualizerLib)

set_target_properties(test 
    PROPERTIES 
    LINKER_LANGUAGE CXX
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${TARGET_INSTALL_AREA}/bin"
)

add_executable(main 
    core/main.cpp
    core/Application.cpp
    Window/glWindow.cpp
    Window/GUIWindows.cpp
    Window/Window.cpp
    OpenGL/Scene/Detector.cpp
    OpenGL/Scene/Mesh.cpp
    OpenGL/Camera.cpp
    OpenGL/Framebuffer.cpp
    OpenGL/Renderer.cpp
    OpenGL/Shader.cpp
)

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(main VisualizerLib)

set_target_properties(main 
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
