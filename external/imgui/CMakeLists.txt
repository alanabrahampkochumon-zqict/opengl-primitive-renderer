add_library(imgui STATIC
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui.h

        # GLFW backend
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_glfw.h

        # OpenGL3 backend
        backends/imgui_impl_opengl3.cpp
        backends/imgui_impl_opengl3.h
)

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/backends)

target_link_libraries(imgui PRIVATE glfw)