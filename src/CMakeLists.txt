ADD_EXTERNAL_DEPS()

add_library(VisualizerLib SHARED
    datasets/YarrBinaryFile.cpp
    datasets/SocketReceiver.cpp
    datasets/SocketSubscriber.cpp
    datasets/AllDataLoaders.cpp
    datasets/EventReconstructor.cpp
    datasets/CorryWrapper.cpp
    util/include/json.hpp
    util/include/util.hpp
    util/include/cli.h
    util/include/logging.h
    util/include/CircularBuffer.h
    util/include/FEBookie.h
    util/FEBookie.cpp
    util/logging.cpp
    util/cli.cpp
    util/util.cpp
)

target_link_libraries(VisualizerLib PUBLIC pthread rt glfw glad imgui glm stb libzmq corry_lib)
target_include_directories(VisualizerLib PUBLIC ${PROJECT_SOURCE_DIR}/src 
                                                datasets/include
                                                util/include)

 add_executable(main
    core/main.cpp
    core/Application.cpp
    Window/glWindow.cpp
    Window/GUIWindows.cpp
    Window/Window.cpp
    OpenGL/Scene/Detector.cpp
    OpenGL/Scene/Mesh.cpp
    OpenGL/Scene/Chip.cpp
    OpenGL/Scene/Curve.cpp
    OpenGL/Scene/Skybox.cpp
    OpenGL/Object/Cube.cpp
    OpenGL/Camera.cpp
    OpenGL/Framebuffer.cpp
    OpenGL/Renderer.cpp
    OpenGL/Shader.cpp
    OpenGL/ShaderManager.cpp
    OpenGL/Texture.cpp
    OpenGL/Cubemap.cpp
    )

target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(main VisualizerLib)

set_target_properties(main 
    PROPERTIES 
    LINKER_LANGUAGE CXX
    # ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    # LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_INSTALL_AREA}
)

add_executable(datatest
    core/datareader_test.cpp
)
target_link_libraries(datatest VisualizerLib pthread)
set_target_properties(datatest
    PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_INSTALL_AREA}
)

#add_executable(reconstruction
#    core/EventReconstructionTest.cpp
#)
#target_include_directories(reconstruction PUBLIC ${PROJECT_SOURCE_DIR}/src)
#target_link_libraries(reconstruction VisualizerLib pthread)
#set_target_properties(reconstruction
#    PROPERTIES
#    LINKER_LANGUAGE CXX
#    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_INSTALL_AREA}
#)

add_executable(trackingtest
    core/CorryWrapperTest.cpp
)
target_include_directories(trackingtest PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(trackingtest VisualizerLib pthread)
set_target_properties(trackingtest
    PROPERTIES
    LINKER_LANGUAGE CXX
    RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_INSTALL_AREA}
)

# message("Saving bin files to ${TARGET_INSTALL_AREA}")
