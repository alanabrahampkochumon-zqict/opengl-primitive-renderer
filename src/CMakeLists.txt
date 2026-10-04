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

target_include_directories(OpenGLRenderer PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(OpenGLRenderer PUBLIC
        libglew_static
        glfw
        glm::glm
        imgui
        stb
)
