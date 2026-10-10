option(IMGUI_DEBUG_WINDOW "Turn of Imgui Debug Window" OFF)
add_executable(OpenGLRenderer
        # Source Files
        3dshapes/ShapeMeshes.cpp
        scene/SceneManager.cpp
        scene/ViewManager.cpp
        utils/ShaderManager.cpp
        Main.cpp

        # Headers
        3dshapes/ShapeMeshes.h
        scene/MaterialTag.h
        scene/Mesh.h
        scene/SceneManager.h
        scene/Texture.h
        scene/ViewManager.h
        utils/camera.h
        utils/ShaderManager.h
)


if(IMGUI_DEBUG_WINDOW)
    target_compile_definitions(OpenGLRenderer PRIVATE ENABLE_IMGUI)
endif ()

target_include_directories(OpenGLRenderer PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(OpenGLRenderer PUBLIC
        libglew_static
        glfw
        glm::glm
        imgui
        stb
)

add_custom_command(
        TARGET OpenGLRenderer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res"
        "$<TARGET_FILE_DIR:OpenGLRenderer>"
        COMMENT "Copying resources to build directory..."
)
