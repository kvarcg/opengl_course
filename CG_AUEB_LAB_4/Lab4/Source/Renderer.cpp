//----------------------------------------------------//
//                                                    //
// File: Renderer.cpp                                 //
// Initialization, rendering and input handling       //
// happens here                                       //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

#include "HelpLib.h"    // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.

#include "OBJLoader.h"  // - Header file for the OBJ Loader
#include "OGLMesh.h"    // - Header file for the OGL mesh
#include "ShaderGLSL.h" // - Header file for GLSL objects
#include "Renderer.h"   // - Header file for our OpenGL functions

#include "SceneGraph/Root.h"
#include "SceneGraph/TransformNode.h"
#include "SceneGraph/GroupNode.h"
#include "SceneGraph/GeometryNode.h"

// camera parameters
glm::vec3 eye;
glm::vec3 target;
glm::vec3 up;

// transformation matrices
glm::mat4x4 world_to_camera_matrix;
glm::mat4x4 perspective_projection_matrix;

// Mouse button and coords
int mouse_button = -1;
int prev_x = 0;
int prev_y = 0;

// World Rotation
float world_rotate_x = 0;
glm::vec3 world_translate;

// object rotations
float planet1_angle = 0.0f;

// vertex array objects
GLuint vao_sphere;
GLint vao_sphere_indices;

// Rendering mode
int rendering_mode = GL_FILL;

// OBJ models
OGLMesh* sphereMesh;

// Scene graph nodes
Root* root;
TransformNode* world_transform;
TransformNode* planet1_transform;
TransformNode* planet2_transform;
GeometryNode* planet1_geom;
GeometryNode* planet2_geom;

// forward declarations
bool LoadObjModels();
void SceneGraphExampleInit();
void SceneGraphExampleDraw();

// This init function is called before FREEGLUT goes into its main loop.
bool InitializeRenderer(void)
{
    // Initialize GLEW (openGL extensions)
    // GLEW is a library which determines which OpenGL extensions are supported on each platform
    // For example, in windows we need this since Microsoft does not support anything beyond OpenGL 1.1 (look at the gl/gl.h header file)
    // The code below uses GLEW to include any extensions supported in this platform
    glewExperimental = GL_TRUE;
    GLuint err = glewInit();
    // skip GLEW bug in glewInit
    unsigned int _enum = glGetError();
    if (GLEW_OK != err)
    {
        PrintToOutputWindow("GLEW error %s", glewGetErrorString(err));
        return false;
    }
    else
        PrintToOutputWindow("Using GLEW %s", glewGetString(GLEW_VERSION));

    // check specifically for OpenGL 3.3 support
    bool ogl33_supported = (bool)(GLEW_GET_VAR(__GLEW_VERSION_3_3)!=0);
    if (!ogl33_supported)
    {
        PrintToOutputWindow("OpenGL 3.3 not supported. Exiting.");
        return false;
    }
    else
        PrintToOutputWindow("OpenGL 3.3 supported.");

    // this is where the variables that will point to the vertex array objects (VAO) are initialized
    vao_sphere = 0;

    // for the camera
    eye = glm::vec3(0.0f, 0.0f, 100.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    world_translate = glm::vec3(0);

    // obj model loading goes here
    if (!LoadObjModels())
    {
        PrintToOutputWindow("Error in loading obj models. Exiting");
        return false;
    }

    // Enable the depth buffer
    // Default is disabled
    glDisable(GL_DEPTH_TEST);

    // glDepthFunc tells OpenGL to accept fragments whose depth is less or equal than the current depth value
    // Note that 0.0 denotes the near field and 1.0 denotes the far field
    // Initial value is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // Set the RGBA values to clear the color buffer (using glClear later on)
    glClearColor(0.2f, 0.2f, 0.25f, 1.0f);

    // Set the value to clear the depth buffer to value 1.0 (all far) (using glClear later on)
    glClearDepth(1.0f);

    // Enable Face Culling
    glEnable(GL_CULL_FACE);
    // Tell OpenGL to cull back faces
    glCullFace(GL_BACK);
    // Tell OpenGL that the front faces are the ones in counter-clockwise order
    glFrontFace(GL_CCW);
    // Enable the depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable Scene Graph
    /*
    SceneGraphExampleInit();
    */

    // check if we have generated any OpenGL errors
    glError();

    return true;
}

void SceneGraphExampleInit()
{
    // Scene graph example
    // eye settings
    // eye = glm::vec3(0.0f, 0.0f, 100.0f);
    // Similar to planetary system (note however that the parent transform that is applied now is T * R * S)
    root = new Root();
    world_transform = new TransformNode("world_transform");
    planet1_transform = new TransformNode("planet1_transform");
    planet2_transform = new TransformNode("planet2_transform");
    planet1_geom = new GeometryNode("planet1_geom", sphereMesh);
    planet2_geom = new GeometryNode("planet2_geom", sphereMesh);

    // construct the render tree
    root->AddChild(world_transform);
    world_transform->AddChild(planet1_transform);
    planet1_transform->AddChild(planet1_geom);
    planet1_transform->AddChild(planet2_transform);
    planet2_transform->AddChild(planet2_geom);

    // initialize root (sets the parent->child relationships in the tree)
    root->Init();
}

bool LoadObjModels()
{
    OBJLoader* objLoader = new OBJLoader();
    sphereMesh = objLoader->loadMesh("sphere.obj", "..\\..\\Data\\Other");

    return true;
}

// Render function. Every time our window has to be drawn, this is called.
void Render(void)
{
    // First this, we clear our depth and color buffers.
    // We can clear both using an | operand to create the buffer-clear mask.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the rendering mode
    glPolygonMode(GL_FRONT_AND_BACK, rendering_mode);

    // CAMERA SETUP
    world_to_camera_matrix = glm::lookAt(eye, target, up);

    // Enable Scene Graph
    /*
    SceneGraphExampleDraw();
    */
    // Remember that we are using double-buffering, all the drawing we just did
    // took place in the "hidden" back-buffer. Calling glutSwapBuffers makes the
    // back buffer "visible".
    glutSwapBuffers();

    glError();
}

// Release all memory allocated by pointers using new
void ReleaseGLUT()
{

}

void SceneGraphExampleDraw()
{
    if (root == nullptr) return;

    // USE SCENE GRAPH
    root->SetViewMat(world_to_camera_matrix);
    root->SetProjectionMat(perspective_projection_matrix);

    // update transformation nodes
    // random rotation offsets
    // this check is so that each time we exceed 360 degrees we go back to 0 (avoids overflow)
    if (planet1_angle > 360.0f) planet1_angle -= 360.0f;

    planet1_angle += 0.4f;

    // world transformations
    world_transform->SetTranslation(world_translate.x, world_translate.y, world_translate.z);
    world_transform->SetRotation(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // PLANET 1
    planet1_transform->SetScale(3.0f, 3.0f, 3.0f);
    planet1_transform->SetRotation(planet1_angle, 0.0f, 1.0f, 0.0f);

    // PLANET 2
    planet2_transform->SetScale(0.2f, 0.2f, 0.2f);
    planet2_transform->SetTranslation(5.0f, 2.0f, 0.0f);

    // Draw the scene by traversing the scene graph
    root->Draw();
}

// Keyboard callback function.
// When a key is pressed, GLUT calls this, passing the keys character in the key parameter.
// The x,y values are the window mouse coordinates when the key was pressed
void Keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 'w':
    case 'W':
        eye.z -= 1.0f;
        break;
    case 's':
    case 'S':
        eye.z += 1.0f;
        break;
    case 'a':
    case 'A':
        eye.x -= 1.0f;
        break;
    case 'd':
    case 'D':
        eye.x += 1.0f;
        break;
    case 'r':
    case 'R':
        eye.y += 1.0f;
        break;
    case 'f':
    case 'F':
        eye.y -= 1.0f;
        break;
    case 27: // escape
        glutLeaveMainLoop();
        return;
    default:
        return;
    }
    // since we have changed some of our parameters, redraw!
    glutPostRedisplay();
}

// Special Keyboard callback function.
// When a special key is pressed (such as arrows or function keys), GLUT calls this, passing the keys character in the key parameter.
// The x,y values are the window mouse coordinates when the key was pressed
void KeyboardSpecial(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_F1:
        if (rendering_mode == GL_POINT) rendering_mode = GL_LINE;
        else if (rendering_mode == GL_LINE) rendering_mode = GL_FILL;
        else if (rendering_mode == GL_FILL) rendering_mode = GL_POINT;
    case GLUT_KEY_F2:
        break;
    case GLUT_KEY_PAGE_UP:
        world_translate.y += 1.0f;
        break;
    case GLUT_KEY_PAGE_DOWN:
        world_translate.y -= 1.0f;
        break;
    case GLUT_KEY_UP:
        world_translate.z += 1.0f;
        break;
    case GLUT_KEY_DOWN:
        world_translate.z -= 1.0f;
        break;
    case GLUT_KEY_LEFT:
        world_translate.x -= 1.0f;
        break;
    case GLUT_KEY_RIGHT:
        world_translate.x += 1.0f;
        break;
    default:
        return;
    }
    // since we have changed some of our parameters, redraw!
    glutPostRedisplay();
}

// This function redraws the scene every 16ms
void TimerSync(int ms)
{
    glutPostRedisplay();
    glutTimerFunc(16, TimerSync, 16);
}

// Mouse callback function
// button refers to the mouse button, state refers to whether button is in up or down state
// and x, y are the new mouse coordinates
void Mouse(int button, int state, int x, int y)
{
    //OutputDebugString("Entered Mouse\n");
    if (state == GLUT_DOWN)
    {
        prev_x = x;
        prev_y = y;
    }
    mouse_button = button;
}

// Method to handle the mouse motion
void MouseMotion(int x, int y)
{
    //OutputDebugString("Entered Mouse Motion\n");
    switch (mouse_button)
    {
    case GLUT_LEFT_BUTTON:
        target.x -= (x - prev_x) * 0.05f;
        target.y += (y - prev_y) * 0.05f;

        prev_x = x;
        prev_y = y;
        glutPostRedisplay();
        break;
    case GLUT_RIGHT_BUTTON:

        world_rotate_x += (x - prev_x) * 0.1f;

        prev_x = x;
        prev_y = y;

        glutPostRedisplay();
        break;
    default:
        break;
    }
}

// Resize callback function
// The parameters indicate the new width and height.
void Resize(int width, int height)
{
    // Hack to void zero height
    if (height == 0) height = 1;


    // -------------------------------------------VIEWPORT----------------------------------------------//
    // Set up the viewport (the part of the window where the rendering happens)
    // 0, 0 are the coordinates at the LOWER LEFT corner of the viewport rectangle, in pixels
    // Width, height Specify the width and height of the viewport, in pixels.
    // Typically, this is the size of the window
    // This information will be used for the viewport transformation
    glViewport(0, 0, width, height);
    // -------------------------------------------------------------------------------------------------//


    // aspect ratio of the window
    float aspect_ratio = (float) width / (float) height;
    // value for the near clipping plane. Anything before that will be clipped
    float near_field_value = 1.0f;
    // value for the far clipping plane. Anything beyond that will be clipped
    float far_field_value = 1000.0f;

    // --------------------------------PERSPECTIVE PROJECTION-------------------------------------------//
    // each time we resize the window, a new projection matrix must be defined
    // use GLM to create a perspective projection matrix (good for 3D rendering)
    // vertical field of view angle
    float vertical_FOV = 30.0f;

    // use GLM to create a perspective projection matrix
    perspective_projection_matrix = glm::perspective(vertical_FOV, aspect_ratio, near_field_value, far_field_value);

    // -------------------------------------------------------------------------------------------------//
}