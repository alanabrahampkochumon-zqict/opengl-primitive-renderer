#include <cstdlib>  // EXIT_FAILURE
#include <iostream> // error handling and output

// IMGUI Headers
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include <GL/glew.h> // GLEW library
#include <GLFW/glfw3.h> // GLFW

// GLM Math Header inclusions
#include "3dshapes/ShapeMeshes.h"
#include "scene/SceneManager.h"
#include "scene/ViewManager.h"
#include "utils/ShaderManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// Namespace for declaring global variables
namespace
{
    // Macro for window title
    const char* const WINDOW_TITLE = "OpenGL Renderer";

    // Main GLFW window
    GLFWwindow* g_Window = nullptr;

    // scene manager object for managing the 3D scene prepare and render
    SceneManager* g_SceneManager = nullptr;
    // shader manager object for dynamic interaction with the shader code
    ShaderManager* g_ShaderManager = nullptr;
    // view manager object for managing the 3D view setup and projection to 2D
    ViewManager* g_ViewManager = nullptr;
} // namespace

// Function declarations - all functions that are called manually
// need to be pre-declared at the beginning of the source code.
bool InitializeGLFW();
bool InitializeGLEW();


inline void imguiInit(GLFWwindow* window)
{
    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + generally GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + generally GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif


    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting
    // Style + calling this again)
    style.FontScaleDpi = main_scale;
    // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for
    // every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

inline void imguiBeginRender(GLFWwindow* window, const ImGuiIO& io)
{
    // IMGUI State
    bool show_demo_window = false;

    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

inline void imguiEndRender(GLFWwindow* window)
{
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiCleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

/***********************************************************
 *  main(int, char*)
 *
 *  This function gets called after the application has been
 *  launched.
 ***********************************************************/
int main(int argc, char* argv[])
{
    // if GLFW fails initialization, then terminate the application
    if (InitializeGLFW() == false)
    {
        return (EXIT_FAILURE);
    }

    // try to create a new shader manager object
    g_ShaderManager = new ShaderManager();
    // try to create a new view manager object
    g_ViewManager = new ViewManager(g_ShaderManager);

    // try to create the main display window
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);


    //// IMGUI INIT
    imguiInit(g_Window);
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    //// IMGUI INIT

    // if GLEW fails initialization, then terminate the application
    if (InitializeGLEW() == false)
    {
        return (EXIT_FAILURE);
    }

    // load the shader code from the external GLSL files
    g_ShaderManager->LoadShaders("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    // try to create a new scene manager object and prepare the 3D scene
    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // loop will keep running until the application is closed
    // or until an error has occurred
    while (!glfwWindowShouldClose(g_Window))
    {

        // query the latest GLFW events
        glfwPollEvents();

        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear the frame and z buffers
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // convert from 3D object space to 2D view
        g_ViewManager->PrepareSceneView();

        // Draw the imgui window
        imguiBeginRender(g_Window, io);

        // refresh the 3D scene
        g_SceneManager->RenderScene();

        // End the renderpass of imgui
        imguiEndRender(g_Window);

        // Flips the the back buffer with the front buffer every frame.
        glfwSwapBuffers(g_Window);
    }

    // clear the allocated manager objects from memory
    if (NULL != g_SceneManager)
    {
        delete g_SceneManager;
        g_SceneManager = NULL;
    }
    if (NULL != g_ViewManager)
    {
        delete g_ViewManager;
        g_ViewManager = NULL;
    }
    if (NULL != g_ShaderManager)
    {
        delete g_ShaderManager;
        g_ShaderManager = NULL;
    }

    imguiCleanup();

    // Terminates the program successfully
    exit(EXIT_SUCCESS);
}

/***********************************************************
 *	InitializeGLFW()
 *
 *  This function is used to initialize the GLFW library.
 ***********************************************************/
bool InitializeGLFW()
{
    // GLFW: initialize and configure library
    // --------------------------------------
    glfwInit();

#ifdef __APPLE__
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // set the version of OpenGL and profile to use
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    // GLFW: end -------------------------------

    return (true);
}

/***********************************************************
 *	InitializeGLEW()
 *
 *  This function is used to initialize the GLEW library.
 ***********************************************************/
bool InitializeGLEW()
{
    // GLEW: initialize
    // -----------------------------------------
    GLenum GLEWInitResult = GLEW_OK;

    // try to initialize the GLEW library
    GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }
    // GLEW: end -------------------------------

    // Displays a successful OpenGL initialization message
    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;

    return (true);
}
