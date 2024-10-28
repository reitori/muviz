cmake_minimum_required(VERSION 3.0)
project(ImGUI)

add_library(imgui 
    include/imgui.h 
    src/imgui.cpp
    include/imconfig.h
    src/imgui_draw.cpp
    include/imgui_internal.h
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    include/imstb_rectpack.h
    include/imstb_textedit.h
    include/imstb_truetype.h
    src/imgui_impl_glfw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_demo.cpp
    include/imgui_impl_glfw.h
    include/imgui_impl_opengl3.h
    include/imgui_impl_opengl3_loader.h
)

target_link_libraries(imgui glfw)
target_include_directories(imgui PUBLIC include/)
target_include_directories(imgui PRIVATE src/)