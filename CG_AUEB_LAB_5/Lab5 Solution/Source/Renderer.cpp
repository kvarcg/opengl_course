﻿//----------------------------------------------------//
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
#include "Light.h"      // - Header file for Lights
#include "Shaders.h"    // - Header file for all the shaders

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

// Rendering mode
int rendering_mode = GL_FILL;

// OBJ models
OGLMesh* groundwhiteMesh;
OGLMesh* spherewhiteMesh;
OGLMesh* sphereredMesh;
OGLMesh* sphereblueMesh;
OGLMesh* sphereyellowaMesh;
OGLMesh* spheregreenaMesh;
OGLMesh* treasureMesh;
OGLMesh* skeletonMesh;

// Scene graph nodes
Root* root;
TransformNode* world_transform;
// for scene 1
TransformNode* ground_transform;
TransformNode* sphere1_transform;
TransformNode* sphere2_transform;
GeometryNode* ground_geom;
GeometryNode* sphere1_geom;
GeometryNode* sphere2_geom;
// for scene 2
TransformNode* treasure_transform;
TransformNode* skeleton_transform;
GeometryNode* treasure_geom;
GeometryNode* skeleton_geom;

// Shaders
// omni light shader
OmniLightShader* omnilight_shader;
// directional light shader
DirectionalLightShader* directionallight_shader;
// basic geometry shader
BasicGeometryShader* basic_geometry_shader;

// Lights
DirectionalLight* sunlight;
OmniLight* candlelight;

// light parameters (for animating the light)
float sunlight_rotationY;
float candlelight_rotationY;

// forward declarations
bool CreateShaders();
bool LoadObjModels();
void DrawOGLMesh(OGLMesh* mesh, glm::mat4x4& object_to_world_transform);
void BlendingExampleDraw();
void SceneGraphExampleInit();
void SceneGraphExampleDraw();
void SceneGraphExample2Init();
void SceneGraphExample2Draw();

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

    // define the lights here
    sunlight = new DirectionalLight();
    sunlight->m_name = "sun";
    sunlight->m_color = glm::vec3(1.0f, 1.0f, 1.0f);
    sunlight->m_initial_direction = glm::vec4(1.0f, -1.0f, 0.0f, 0.0f);

    candlelight = new OmniLight();
    candlelight->m_name = "candle";
    candlelight->m_color = glm::vec3(1.0f, 0.57f, 0.16f);
    candlelight->m_initial_position = glm::vec4(10.0f, 10.0f, 0.0f, 1.0f);

    // for the camera
    eye = glm::vec3(0.0f, 0.0f, 100.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    world_translate = glm::vec3(0);

    // Create the shaders here
    // Shader declaration is in the Shaders.h file
    // The CreateShaders function initializes the pointers, compiles the shader files
    // and checks for uniforms
    if (!CreateShaders())
    {
        return false;
    }

    // Load the OBJ models here
    if (!LoadObjModels())
    {
        PrintToOutputWindow("Error in loading obj models. Exiting");
        return false;
    }

    // Enable the depth buffer
    // Default is disabled
    glEnable(GL_DEPTH_TEST);

    // glDepthFunc tells OpenGL to accept fragments whose depth is less or equal than the current depth value
    // Note that 0.0 denotes the near field and 1.0 denotes the far field
    // Initial value is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // Set the RGBA values to clear the color buffer (using glClear later on)
    //glClearColor(0.08f, 0.17f, 0.2f, 1.0f);
    glClearColor(0.03f, 0.2f, 0.22f, 1.0f);

    // Set the value to clear the depth buffer to value 1.0 (all far) (using glClear later on)
    glClearDepth(1.0f);

    // Enable Face Culling
    glEnable(GL_CULL_FACE);
    // Tell OpenGL to cull back faces
    glCullFace(GL_BACK);
    // Tell OpenGL that the front faces are the ones in counter-clockwise order
    glFrontFace(GL_CCW);

    // Enable Scene Graph

    SceneGraphExampleInit();

    /*
    SceneGraphExample2Init();
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
    // Similar to sphereary system (note however that the parent transform that is applied now is T * R * S)
    root = new Root();
    world_transform = new TransformNode("world_transform");
    ground_transform = new TransformNode("ground_transform");
    sphere1_transform = new TransformNode("sphere1_transform");
    sphere2_transform = new TransformNode("sphere2_transform");
    ground_geom = new GeometryNode("ground_geom", groundwhiteMesh);
    sphere1_geom = new GeometryNode("sphere1_geom", spherewhiteMesh);
    sphere2_geom = new GeometryNode("sphere2_geom", spherewhiteMesh);

    // construct the render tree
    root->AddChild(world_transform);
    world_transform->AddChild(ground_transform);
    world_transform->AddChild(sphere1_transform);
    ground_transform->AddChild(ground_geom);
    sphere1_transform->AddChild(sphere1_geom);
    sphere1_transform->AddChild(sphere2_transform);
    sphere2_transform->AddChild(sphere2_geom);

    // initialize root (sets the parent->child relationships in the tree)
    root->Init();

    // since we are using more than one shaders, we are passing them to the root node
    // so they can be accessible from the geometry node during the Draw call
    // this is because each shader requires different uniform variables
    root->SetOmniLightShader(omnilight_shader);
    root->SetDirectionalLightShader(directionallight_shader);
}

void SceneGraphExample2Init()
{
    // Scene graph example 2
    // eye settings
    // eye = glm::vec3(80.0f, 60.0f, 100.0f);
    // A skeleton and a chest
    root = new Root();
    world_transform = new TransformNode("world_transform");
    ground_transform = new TransformNode("ground_transform");
    skeleton_transform = new TransformNode("skeleton_transform");
    treasure_transform = new TransformNode("treasure_transform");
    ground_geom = new GeometryNode("ground_geom", groundwhiteMesh);
    skeleton_geom = new GeometryNode("skeleton_geom", skeletonMesh);
    treasure_geom = new GeometryNode("treasure_geom", treasureMesh);

    // construct the render tree
    root->AddChild(world_transform);
    world_transform->AddChild(ground_transform);
    world_transform->AddChild(skeleton_transform);
    world_transform->AddChild(treasure_transform);
    ground_transform->AddChild(ground_geom);
    skeleton_transform->AddChild(skeleton_geom);
    treasure_transform->AddChild(treasure_geom);

    // initialize root (sets the parent->child relationships in the tree)
    root->Init();

    // since we are using more than one shaders, we are passing them to the root node
    // so they can be accessible from the geometry node during the Draw call
    // this is because each shader requires different uniform variables
    root->SetOmniLightShader(omnilight_shader);
    root->SetDirectionalLightShader(directionallight_shader);

    // it's better to apply any static transformations here since
    // they only need to be set once
    treasure_transform->SetScale(0.5f, 0.5f, 0.5f);
    treasure_transform->SetRotation(180.0f, 0.0f, 1.0f, 0.0f);
    treasure_transform->SetTranslation(0.0f, 2.0f, 0.0f);
    skeleton_transform->SetRotation(180.0f, 0.0f, 1.0f, 0.0f);
    skeleton_transform->SetTranslation(0.0f, 0.0f, -18.0f);
    ground_transform->SetTranslation(0.0f, -10.0f, 0.0f);
}

bool LoadObjModels()
{
    OBJLoader* objLoader = new OBJLoader();
    // for the blending example
    sphereredMesh = objLoader->loadMesh("sphere_red.obj", "..\\..\\Data\\Other");
    sphereblueMesh = objLoader->loadMesh("sphere_blue.obj", "..\\..\\Data\\Other");
    sphereyellowaMesh = objLoader->loadMesh("sphere_yellow_with_alpha.obj", "..\\..\\Data\\Other");
    spheregreenaMesh = objLoader->loadMesh("sphere_green_with_alpha.obj", "..\\..\\Data\\Other");

    // for both scenes
    groundwhiteMesh = objLoader->loadMesh("plane_white.obj", "..\\..\\Data\\Other");

    // for scene 1
    spherewhiteMesh = objLoader->loadMesh("sphere_white.obj", "..\\..\\Data\\Other");

    // for scene 2
    treasureMesh = objLoader->loadMesh("treasure.obj", "..\\..\\Data\\Pirates");
    skeletonMesh = objLoader->loadMesh("skeleton.obj", "..\\..\\Data\\Pirates");

    return true;
}

bool CreateShaders()
{
    // Basic Geometry Shader (the shader with no lighting calculations, similar to the previous labs)
    // The only difference here is that we are storing the shader pointer, the program id and the uniforms in
    // a struct instead of using global variables
    // All the shaders below are located in the Shaders.h file

    // Basic Geometry shader
    basic_geometry_shader = new BasicGeometryShader();
    basic_geometry_shader->shader = new ShaderGLSL("BasicGeometry");
    // compile
    bool shader_loaded = basic_geometry_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    basic_geometry_shader->program_id = basic_geometry_shader->shader->GetProgram();
    // check for uniforms
    basic_geometry_shader->uniform_m = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_m");
    basic_geometry_shader->uniform_v = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_v");
    basic_geometry_shader->uniform_p = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_p");
    basic_geometry_shader->uniform_color = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_material_color");

    // Omni light shader
    // This is used for rendering geometry using an omnidirectional light shader
    omnilight_shader = new OmniLightShader();
    omnilight_shader->shader = new ShaderGLSL("OmniLight");
    // compile
    shader_loaded = omnilight_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    omnilight_shader->program_id = omnilight_shader->shader->GetProgram();
    // check for uniforms
    omnilight_shader->uniform_m = glGetUniformLocation(omnilight_shader->program_id, "uniform_m");
    omnilight_shader->uniform_v = glGetUniformLocation(omnilight_shader->program_id, "uniform_v");
    omnilight_shader->uniform_p = glGetUniformLocation(omnilight_shader->program_id, "uniform_p");
    omnilight_shader->uniform_color = glGetUniformLocation(omnilight_shader->program_id, "uniform_material_color");
    omnilight_shader->uniform_normal_matrix_ecs = glGetUniformLocation(omnilight_shader->program_id, "uniform_normal_matrix_ecs");
    omnilight_shader->uniform_light_color = glGetUniformLocation(omnilight_shader->program_id, "uniform_light_color");
    omnilight_shader->uniform_light_position_ecs = glGetUniformLocation(omnilight_shader->program_id, "uniform_light_position_ecs");

    // Directional light shader
    // This is used for rendering geometry using a directional light shader
    directionallight_shader = new DirectionalLightShader();
    directionallight_shader->shader = new ShaderGLSL("DirectionalLight");
    // compile
    shader_loaded = directionallight_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    directionallight_shader->program_id = directionallight_shader->shader->GetProgram();
    // check for uniforms
    directionallight_shader->uniform_m = glGetUniformLocation(directionallight_shader->program_id, "uniform_m");
    directionallight_shader->uniform_v = glGetUniformLocation(directionallight_shader->program_id, "uniform_v");
    directionallight_shader->uniform_p = glGetUniformLocation(directionallight_shader->program_id, "uniform_p");
    directionallight_shader->uniform_color = glGetUniformLocation(directionallight_shader->program_id, "uniform_material_color");
    directionallight_shader->uniform_normal_matrix_ecs = glGetUniformLocation(directionallight_shader->program_id, "uniform_normal_matrix_ecs");
    directionallight_shader->uniform_light_color = glGetUniformLocation(directionallight_shader->program_id, "uniform_light_color");
    directionallight_shader->uniform_light_direction_ecs = glGetUniformLocation(directionallight_shader->program_id, "uniform_light_direction_ecs");

    // all shaders loaded OK
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

    // we also need to build the world transform (since we can rotate the scene with the mouse or move the world with the arrow keys)
    glm::mat4x4 wld = glm::translate(world_translate) * glm::rotate(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // change the rotation angle for the lights
    // this allows for the lights to be rotated
    sunlight_rotationY += 3.4f;
    if (sunlight_rotationY > 360.0f) sunlight_rotationY -= 360.0f;
    candlelight_rotationY += 0.8f;
    if (candlelight_rotationY > 360.0f) candlelight_rotationY -= 360.0f;

    sunlight->m_transformed_direction = sunlight->m_initial_direction;
    //sunlight->m_transformed_direction = glm::rotate(sunlight_rotationY, 0.0f, 1.0f, 0.0f) * sunlight->m_initial_direction;
    candlelight->m_tranformed_position = candlelight->m_initial_position;
    candlelight->m_tranformed_position = glm::rotate(candlelight_rotationY, 0.0f, 1.0f, 0.0f) * candlelight->m_initial_position;

    glm::mat4x4 obj = glm::translate(glm::vec3(candlelight->m_tranformed_position)) * glm::scale(0.2f, 0.2f, 0.2f);
    obj = wld * obj;
    DrawOGLMesh(spherewhiteMesh, obj);
    candlelight->m_tranformed_position = wld * candlelight->m_tranformed_position;

    // Blending example
    /*
    BlendingExampleDraw();
    */

    // Enable Scene Graph

    SceneGraphExampleDraw();

    /*
    SceneGraphExample2Draw();
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

    // set current active light
    // in this example, the sunlight is set as the active directional light
    // and the candle light as the active omni light
    // to draw using the directional light, we pass 0 to the Draw function
    // and to draw using the omni light, we pass 1 to the Draw function
    // to draw using multiple lights, we simply need to enable blending
    // and use an additive blend function glBlendFunc(GL_ONE, GL_ONE)
    // since lights are additive
    root->SetActiveDirectionalLight(sunlight);
    root->SetActiveOmnilLight(candlelight);

    // world transformations
    world_transform->SetTranslation(world_translate.x, world_translate.y, world_translate.z);
    world_transform->SetRotation(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // Ground
    ground_transform->SetTranslation(0.0f, -10.0f, 0.0f);

    // Sphere 1
    sphere1_transform->SetScale(3.0f, 3.0f, 3.0f);
    sphere1_transform->SetRotation(0, 0.0f, 1.0f, 0.0f);

    // Sphere 2
    sphere2_transform->SetScale(0.4f, 0.4f, 0.4f);
    sphere2_transform->SetTranslation(5.0f, 2.0f, 0.0f);

    // Draw the scene by traversing the scene graph
    // 0 renders the geometry using the directional light shader
    // 1 renders the geometry using the omnidirectional light shader
    root->Draw(1);
}

void SceneGraphExample2Draw()
{
    if (root == nullptr) return;

    // USE SCENE GRAPH
    root->SetViewMat(world_to_camera_matrix);
    root->SetProjectionMat(perspective_projection_matrix);

    // set current active light
    // in this example, the sunlight is set as the active directional light
    // and the candle light as the active omni light
    // to draw using the directional light, we pass 0 to the Draw function
    // and to draw using the omni light, we pass 1 to the Draw function
    // to draw using multiple lights, we simply need to enable blending
    // and use an additive blend function glBlendFunc(GL_ONE, GL_ONE)
    // since lights are additive
    root->SetActiveDirectionalLight(sunlight);
    root->SetActiveOmnilLight(candlelight);

    // world transformations
    world_transform->SetTranslation(world_translate.x, world_translate.y, world_translate.z);
    world_transform->SetRotation(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // Draw the scene by traversing the scene graph
    // 0 renders the geometry using the directional light shader
    // 1 renders the geometry using the omnidirectional light shader
    root->Draw(1);
}

void BlendingExampleDraw(void)
{
    // set world transformation
    glm::mat4x4 world_matrix = glm::mat4x4(1);
    world_matrix = glm::rotate(world_rotate_x, 1.0f, 0.0f, 0.0f);
    glm::mat4x4 parent_transform = world_matrix;

    // Render opaque objects first (red)

    // Sphere 1 (the furthest from the camera)
    glm::mat4x4 sphere1_scale_matrix = glm::scale(2.0f, 2.0f, 2.0f);
    glm::mat4x4 sphere1_translation_matrix = glm::translate(3.0f, 2.0f, -30.0f);
    glm::mat4x4 sphere1_object_to_world_matrix = parent_transform * sphere1_translation_matrix * sphere1_scale_matrix;

    DrawOGLMesh(sphereredMesh, sphere1_object_to_world_matrix);

    // Sphere 2 (the closest to the camera)
    glm::mat4x4 sphere2_scale_matrix = glm::scale(2.0f, 2.0f, 2.0f);
    glm::mat4x4 sphere2_translation_matrix = glm::translate(5.0f, 0.0f, -7.0f);
    glm::mat4x4 sphere2_object_to_world_matrix = parent_transform * sphere2_translation_matrix * sphere2_scale_matrix;
    DrawOGLMesh(sphereblueMesh, sphere2_object_to_world_matrix);

    // enable blending
    glEnable(GL_BLEND);
    // set the blending function to perform linear interpolation
    // based on the source alpha (the current alpha in the fragment)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render objects with transparency in back-to-front order (should be green furthest, yellow closest)
    // Remember, we are looking down the negative Z axis so the furthest object is the one with the largest
    // negative value

    // Sphere 4
    glm::mat4x4 sphere4_scale_matrix = glm::scale(2.0f, 2.0f, 2.0f);
    glm::mat4x4 sphere4_translation_matrix = glm::translate(3.0f, 0.0f, -12.0f);
    glm::mat4x4 sphere4_object_to_world_matrix = parent_transform * sphere4_translation_matrix * sphere4_scale_matrix;
    DrawOGLMesh(spheregreenaMesh, sphere4_object_to_world_matrix);

    // Sphere 3
    glm::mat4x4 sphere3_scale_matrix = glm::scale(2.0f, 2.0f, 2.0f);
    glm::mat4x4 sphere3_translation_matrix = glm::translate(1.0f, 0.0f, -17.0f);
    glm::mat4x4 sphere3_object_to_world_matrix = parent_transform * sphere3_translation_matrix * sphere3_scale_matrix;
    DrawOGLMesh(sphereyellowaMesh, sphere3_object_to_world_matrix);

    // disable blending
    glDisable(GL_BLEND);
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

        world_rotate_x -= (x - prev_x) * 0.1f;

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

// This is the rendering code (taken from the Draw in the GeometryNode) that renders a mesh
// loaded from an OBJ file
// This is used for the Blending example and since this is shown better without any lighting effects
// we only use the Basic Geometry shader
void DrawOGLMesh(OGLMesh* mesh, glm::mat4x4& object_to_world_transform)
{
    // bind the VAO
    glBindVertexArray(mesh->vao);

    // get the world transformation
    glm::mat4x4& M = object_to_world_transform;
    // get the view transformation
    glm::mat4x4& V = world_to_camera_matrix;
    // get the projection transformation
    glm::mat4x4& P = perspective_projection_matrix;

    std::vector<OBJMaterial*>& materials = mesh->materials;

    // set the shader active
    glUseProgram(basic_geometry_shader->program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(basic_geometry_shader->uniform_m, 1, false, &M[0][0]);
    glUniformMatrix4fv(basic_geometry_shader->uniform_v, 1, false, &V[0][0]);
    glUniformMatrix4fv(basic_geometry_shader->uniform_p, 1, false, &P[0][0]);

    // loop through all the elements
    for (GLint i=0; i < mesh->num_elements; i++)
    {
        if (mesh->elements[i].triangles==0)
            continue;

        // Material and texture goes here.
        int mtrIdx = mesh->elements[i].material_index;
        OBJMaterial& cur_material = *materials[mtrIdx];

        // use the material color
        glUniform4f(basic_geometry_shader->uniform_color, cur_material.m_diffuse[0], cur_material.m_diffuse[1], cur_material.m_diffuse[2], cur_material.m_opacity);

        // draw within a range in the index buffer
        glDrawRangeElements(
            GL_TRIANGLES,
            mesh->elements[i].start_index,
            mesh->elements[i].start_index+mesh->elements[i].triangles*3,
            mesh->elements[i].triangles*3,
            GL_UNSIGNED_INT,
            (void*)(mesh->elements[i].start_index*sizeof(GLuint))
            );
    }

    glBindVertexArray(0);
}