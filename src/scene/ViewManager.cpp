///////////////////////////////////////////////////////////////////////////////
// viewmanager.h
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <iso646.h>

// declaration of the global variables and defines
namespace
{
    // Variables for window width and height
    const int WINDOW_WIDTH       = 1000;
    const int WINDOW_HEIGHT      = 800;
    const char* g_ViewName       = "view";
    const char* g_ProjectionName = "projection";

    // camera object used for viewing and interacting with
    // the 3D scene
    Camera* g_pCamera = nullptr;

    // these variables are used for mouse movement processing
    float gLastX     = WINDOW_WIDTH / 2.0f;
    float gLastY     = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // time between current frame and last frame
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // the following variable is false when orthographic projection
    // is off and true when it is on
    bool bOrthographicProjection = false;
} // namespace

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    // default camera view parameters
    const auto Position = glm::vec3(0.0f, 5.0f, 12.0f);
    const auto Front    = glm::vec3(0.0f, -0.5f, -2.0f);
    const auto Up       = glm::vec3(0.0f, 1.0f, 0.0f);
    const auto Zoom     = 80;

    // initialize the member variables
    m_pShaderManager = pShaderManager;
    m_pWindow        = NULL;
    g_pCamera        = new Camera(Position, Up, Front, Zoom);
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
    // free up allocated memory
    m_pShaderManager = NULL;
    m_pWindow        = NULL;
    if (NULL != g_pCamera)
    {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = nullptr;

    // try to create the displayed OpenGL window
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

// tell GLFW to capture all mouse events
// Enabled only when imgui is disabled
#ifndef ENABLE_IMGUI
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif

    // this callback is used to receive mouse moving events
    glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

    // enable blending for supporting tranparent rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;

    return (window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
    // If it's the first mouse event, then we need to cache the values
    // to ensure all subsequent values are calculated from it
    if (gFirstMouse)
    {
        gLastX      = static_cast<float>(xMousePos);
        gLastY      = static_cast<float>(yMousePos);
        gFirstMouse = false;
    }

    // Calculate the x and y offset for camera movement
    const float xOffset = static_cast<float>(xMousePos) - gLastX;
    const float yOffset = static_cast<float>(gLastY - yMousePos); // reversed since y-coordinates go from bottom to top

    // see the current position into the last position variables
    gLastX = static_cast<float>(xMousePos);
    gLastY = static_cast<float>(yMousePos);

    // Move the 3D camera according to the calculated offsets
    g_pCamera->ProcessMouseMovement(xOffset, yOffset);
}


/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
    // close the window if the escape key has been pressed
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_pWindow, true);
    }
    // Up and Down movement(Q and E)
    if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(UP, gDeltaTime);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);
    }

    // Pan Left and Right (A and D)
    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);
    }

    // Move Forward and Back (W and S)
    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);
    }

    // Switch between perspective and orthographic projections
    if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)
    {
        bOrthographicProjection = false;

        // Delete the old camera if it exists
        if (g_pCamera != nullptr)
        {
            delete g_pCamera;
        }

        // Update the camera
        const auto Position = glm::vec3(0.0f, 5.0f, 12.0f);
        const auto Front    = glm::vec3(0.0f, -0.5f, -2.0f);
        const auto Up       = glm::vec3(0.0f, 1.0f, 0.0f);
        const auto Zoom     = 80;
        g_pCamera           = new Camera(Position, Up, Front, Zoom);
    }
    if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)
    {
        bOrthographicProjection = true;

        // Delete the old camera if it exists
        if (g_pCamera != nullptr)
        {
            delete g_pCamera;
        }

        // Update the camera
        const auto Position = glm::vec3(0.0f, 3.0f, 12.0f);
        const auto Up       = glm::vec3(0.0f, 1.0f, 0.0f);
        const auto Pitch    = 0.0f;
        const auto Yaw      = -90.0f;
        g_pCamera           = new Camera(Position, Up, Yaw, Pitch);
    }
}

void ViewManager::ProcessMouseScrollEvents()
{
    // For handling scroll events we to attach a callback
    // and to avoid creating unnecessary member functions we are using a lambda.
    glfwSetScrollCallback(m_pWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
        g_pCamera->ProcessMouseScroll(static_cast<float>(yoffset));
    });
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
    glm::mat4 view;
    glm::mat4 projection;

    // per-frame timing
    float currentFrame = glfwGetTime();
    gDeltaTime         = currentFrame - gLastFrame;
    gLastFrame         = currentFrame;

    // process any keyboard and mouse scroll events that may be waiting in the
    // event queue
    ProcessKeyboardEvents();
    ProcessMouseScrollEvents();

    // get the current view matrix from the camera
    view = g_pCamera->GetViewMatrix();


    // If we are in orthographic mode use the glm::ortho for orthograhpic project
    const float aspect = static_cast<GLfloat>(WINDOW_WIDTH) / static_cast<GLfloat>(WINDOW_HEIGHT);
    if (bOrthographicProjection)
    {
        float orthoSize = 10.0f; // Zoom in orthographic view
        projection      = glm::ortho(-aspect * orthoSize, aspect * orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
    }
    else
    {
        // else use perspective projection
        projection = glm::perspective(glm::radians(g_pCamera->Zoom), aspect, 0.1f, 100.0f);
    }


    // if the shader manager object is valid
    if (NULL != m_pShaderManager)
    {
        // set the view matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ViewName, view);
        // set the view matrix into the shader for proper rendering
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        // set the view position of the camera into the shader for proper rendering
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}
