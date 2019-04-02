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

#include "HelpLib.h"        // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.

#include "OBJ/OBJLoader.h"  // - Header file for the OBJ Loader
#include "OBJ/OGLMesh.h"    // - Header file for the OGL mesh
#include "ShaderGLSL.h"     // - Header file for GLSL objects
#include "Light.h"          // - Header file for Lights
#include "Shaders.h"        // - Header file for all the shaders
#include "Renderer.h"       // - Header file for our OpenGL functions

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
OGLMesh* lightSourceMesh;
OGLMesh* sphereMapMesh;
OGLMesh* sphereEarthMesh;
OGLMesh* treasureMesh;
OGLMesh* skeletonMesh;
OGLMesh* skeletonGroundMesh;
OGLMesh* knossosMesh;

// Scene graph nodes
Root* root;
TransformNode* world_transform;
TransformNode* ground_transform;
GeometryNode* ground_geom;
// for scene 1
TransformNode* sphere1_transform;
TransformNode* sphere2_transform;
GeometryNode* sphere1_geom;
GeometryNode* sphere2_geom;
// for scene 2
TransformNode* treasure_transform;
TransformNode* skeleton_transform;
GeometryNode* treasure_geom;
GeometryNode* skeleton_geom;

// Shaders
// basic geometry shader
BasicGeometryShader* basic_geometry_shader;
// spotlight shader
SpotLightShader* spotlight_shader;
// ambient light shader
AmbientLightShader* ambient_light_shader;

// Lights
glm::vec3  ambient_color;
SpotLight* spotlight_red;
SpotLight* spotlight_blue;

// light parameters (for animating the light)
float light_rotationY;

// forward declarations
bool CreateShaders();
bool LoadObjModels();
void DrawLightSource(OGLMesh* mesh, glm::mat4x4& object_to_world_transform, glm::vec3& light_emissive_color);
void SceneGraphExampleInit();
void SceneGraphExample2Init();
void DrawSpotLightSource();
void SceneGraphDraw();

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
    spotlight_red = new SpotLight();
    spotlight_red->m_name = "spotlight_red";
    spotlight_blue = new SpotLight();
    spotlight_blue->m_name = "spotlight_yellow";

    // for the camera
    eye = glm::vec3(0.0f, 0.0f, 40.0f);
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

    // Set the value to clear the depth buffer to value 1.0 (all far) (using glClear later on)
    glClearDepth(1.0f);

    // Enable Face Culling
    glEnable(GL_CULL_FACE);
    // Tell OpenGL to cull back faces
    glCullFace(GL_BACK);
    // Tell OpenGL that the front faces are the ones in counter-clockwise order
    glFrontFace(GL_CCW);

    // Enable Scene Graph
    /*
    SceneGraphExampleInit();
    */

    SceneGraphExample2Init();

    // check if we have generated any OpenGL errors
    glError();

    return true;
}

void SceneGraphExampleInit()
{
    // Scene graph example
    // background settings
    // Set the RGBA values to clear the color buffer (using glClear later on)
    glClearColor(0.03f, 0.2f, 0.22f, 1.0f); // for spot light

    // eye settings
    eye = glm::vec3(0.0f, 30.0f, 80.0f);

    // light settings
    ambient_color = glm::vec3(0.02, 0.1, 0.15);

    spotlight_red->m_color = glm::vec3(0.5f, 0.0f, 0.0f);
    spotlight_red->m_initial_target = glm::vec3(0.0f, 0.0f, 0.0f);
    spotlight_red->m_initial_position = glm::vec3(15.0f, 20.0f, 0.0f);

    spotlight_blue->m_color = glm::vec3(0.0f, 0.0f, 0.5f);
    spotlight_blue->m_initial_target = glm::vec3(0.0f, 0.0f, 0.0f);
    spotlight_blue->m_initial_position = glm::vec3(-15.0f, 20.0f, 0.0f);

    // scene settings
    root = new Root();
    world_transform = new TransformNode("world_transform");
    ground_transform = new TransformNode("ground_transform");
    sphere1_transform = new TransformNode("sphere1_transform");
    sphere2_transform = new TransformNode("sphere2_transform");
    ground_geom = new GeometryNode("ground_geom", groundwhiteMesh);
    sphere1_geom = new GeometryNode("sphere1_geom", sphereEarthMesh);
    sphere2_geom = new GeometryNode("sphere2_geom", sphereMapMesh);

    // construct the render tree
    root->AddChild(world_transform);
    world_transform->AddChild(ground_transform);
    world_transform->AddChild(sphere1_transform);
    world_transform->AddChild(sphere2_transform);
    ground_transform->AddChild(ground_geom);
    sphere1_transform->AddChild(sphere1_geom);
    sphere2_transform->AddChild(sphere2_geom);

    // initialize root (sets the parent->child relationships in the tree)
    root->Init();

    // since we are using more than one shaders, we are passing them to the root node
    // so they can be accessible from the geometry node during the Draw call
    // this is because each shader requires different uniform variables
    root->SetSpotlightShader(spotlight_shader);
    root->SetAmbientLightShader(ambient_light_shader);

    root->SetAmbientLightColor(ambient_color);

    // it's better to apply any static transformations here since
    // they only need to be set once
    // Ground
    ground_transform->SetTranslation(0.0f, -10.0f, 0.0f);

    // Sphere 1
    sphere1_transform->SetScale(3.0f, 3.0f, 3.0f);
    sphere1_transform->SetTranslation(-10.0f, 0.0f, 0.0f);

    // Sphere 2
    sphere2_transform->SetScale(3.0f, 3.0f, 3.0f);
    sphere2_transform->SetTranslation(10.0f, 0.0f, 0.0f);
}

void SceneGraphExample2Init()
{
    // Scene graph example 2
    // background settings
    // Set the RGBA values to clear the color buffer (using glClear later on)
    glClearColor(0.03f, 0.2f, 0.22f, 1.0f); // for spot light

    // eye settings
    eye = glm::vec3(0.0f, 60.0f, 180.0f);
    target = glm::vec3(0.0f, 20.0f, 0.0f);

    spotlight_red->m_color = glm::vec3(0.8f, 0.2f, 0.16f);
    spotlight_red->m_initial_target = glm::vec3(-5.0f, 0.0f, 0.0f);
    spotlight_red->m_initial_position = glm::vec3(30.0f, 60.0f, 0.0f);

    spotlight_blue->m_color = glm::vec3(0.3f, 0.2f, 0.8f);
    spotlight_blue->m_initial_target = glm::vec3(5.0f, 0.0f, 0.0f);
    spotlight_blue->m_initial_position = glm::vec3(-30.0f, 60.0f, 0.0f);

    // scene settings
    root = new Root();
    world_transform = new TransformNode("world_transform");
    ground_transform = new TransformNode("ground_transform");
    skeleton_transform = new TransformNode("skeleton_transform");
    treasure_transform = new TransformNode("treasure_transform");
    ground_geom = new GeometryNode("ground_geom", skeletonGroundMesh);
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
    root->SetSpotlightShader(spotlight_shader);
    root->SetAmbientLightShader(ambient_light_shader);

    root->SetAmbientLightColor(ambient_color);

    // it's better to apply any static transformations here since
    // they only need to be set once
    treasure_transform->SetScale(0.5f, 0.5f, 0.5f);
    treasure_transform->SetRotation(180.0f, 0.0f, 1.0f, 0.0f);
    treasure_transform->SetTranslation(0.0f, 2.0f, 0.0f);
    skeleton_transform->SetRotation(180.0f, 0.0f, 1.0f, 0.0f);
    skeleton_transform->SetTranslation(0.0f, 0.0f, -18.0f);
    ground_transform->SetScale(10.0f, 10.0f, 10.0f);
    ground_transform->SetTranslation(0.0f, -8.0f, 0.0f);
}

bool LoadObjModels()
{
    OBJLoader* objLoader = new OBJLoader();

    // for all scenes
    lightSourceMesh = objLoader->loadMesh("light_source.obj", "..\\..\\Data\\Other", true);
    groundwhiteMesh = objLoader->loadMesh("plane_white.obj", "..\\..\\Data\\Other", true);

    // for scene 1
    sphereMapMesh = objLoader->loadMesh("sphere_map.obj", "..\\..\\Data\\Other", true);
    sphereEarthMesh = objLoader->loadMesh("sphere_earth.obj", "..\\..\\Data\\Other", true);

    // for scene 2
    skeletonGroundMesh = objLoader->loadMesh("terrain.obj", "..\\..\\Data\\Other", true);
    treasureMesh = objLoader->loadMesh("treasure.obj", "..\\..\\Data\\Pirates", true);
    skeletonMesh = objLoader->loadMesh("skeleton.obj", "..\\..\\Data\\Pirates", true);

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
    basic_geometry_shader->uniform_material_color = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_material_color");
    basic_geometry_shader->uniform_emissive_color = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_emissive_color");

    // these are for the samplers
    basic_geometry_shader->uniform_sampler_diffuse = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_sampler_diffuse");
    basic_geometry_shader->uniform_has_sampler_diffuse = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_has_sampler_diffuse");

    // Ambient Light shader
    ambient_light_shader = new AmbientLightShader();
    // This is used for rendering geometry using a default ambient light
    ambient_light_shader->shader = new ShaderGLSL("AmbientShader");
    // compile
    shader_loaded = ambient_light_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    ambient_light_shader->program_id = ambient_light_shader->shader->GetProgram();
    // check for uniforms
    ambient_light_shader->uniform_m = glGetUniformLocation(ambient_light_shader->program_id, "uniform_m");
    ambient_light_shader->uniform_v = glGetUniformLocation(ambient_light_shader->program_id, "uniform_v");
    ambient_light_shader->uniform_p = glGetUniformLocation(ambient_light_shader->program_id, "uniform_p");
    ambient_light_shader->uniform_material_color = glGetUniformLocation(ambient_light_shader->program_id, "uniform_material_color");
    ambient_light_shader->uniform_ambient_light_color = glGetUniformLocation(ambient_light_shader->program_id, "uniform_ambient_light_color");

    // these are for the samplers
    ambient_light_shader->uniform_sampler_diffuse = glGetUniformLocation(ambient_light_shader->program_id, "uniform_sampler_diffuse");
    ambient_light_shader->uniform_has_sampler_diffuse = glGetUniformLocation(ambient_light_shader->program_id, "uniform_has_sampler_diffuse");

    // Spotlight light shader
    // This is used for rendering geometry using a spotlight shader
    spotlight_shader = new SpotLightShader();
    spotlight_shader->shader = new ShaderGLSL("SpotLight");
    // compile
    shader_loaded = spotlight_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    spotlight_shader->program_id = spotlight_shader->shader->GetProgram();
    // check for uniforms
    spotlight_shader->uniform_m = glGetUniformLocation(spotlight_shader->program_id, "uniform_m");
    spotlight_shader->uniform_v = glGetUniformLocation(spotlight_shader->program_id, "uniform_v");
    spotlight_shader->uniform_p = glGetUniformLocation(spotlight_shader->program_id, "uniform_p");
    spotlight_shader->uniform_material_color = glGetUniformLocation(spotlight_shader->program_id, "uniform_material_color");
    spotlight_shader->uniform_normal_matrix_ecs = glGetUniformLocation(spotlight_shader->program_id, "uniform_normal_matrix_ecs");
    spotlight_shader->uniform_light_color = glGetUniformLocation(spotlight_shader->program_id, "uniform_light_color");
    spotlight_shader->uniform_light_position_ecs = glGetUniformLocation(spotlight_shader->program_id, "uniform_light_position_ecs");
    spotlight_shader->uniform_light_direction_ecs = glGetUniformLocation(spotlight_shader->program_id, "uniform_light_direction_ecs");

    // these are for the samplers
    spotlight_shader->uniform_sampler_diffuse = glGetUniformLocation(spotlight_shader->program_id, "uniform_sampler_diffuse");
    spotlight_shader->uniform_sampler_normal = glGetUniformLocation(spotlight_shader->program_id, "uniform_sampler_normal");
    spotlight_shader->uniform_sampler_specular = glGetUniformLocation(spotlight_shader->program_id, "uniform_sampler_specular");
    spotlight_shader->uniform_sampler_emission = glGetUniformLocation(spotlight_shader->program_id, "uniform_sampler_emission");
    spotlight_shader->uniform_has_sampler_diffuse = glGetUniformLocation(spotlight_shader->program_id, "uniform_has_sampler_diffuse");
    spotlight_shader->uniform_has_sampler_normal = glGetUniformLocation(spotlight_shader->program_id, "uniform_has_sampler_normal");
    spotlight_shader->uniform_has_sampler_specular = glGetUniformLocation(spotlight_shader->program_id, "uniform_has_sampler_specular");
    spotlight_shader->uniform_has_sampler_emission = glGetUniformLocation(spotlight_shader->program_id, "uniform_has_sampler_emission");

    // all shaders loaded OK
    return true;
}

// Render function. Every time our window has to be drawn, this is called.
void Render(void)
{
    // Set the rendering mode
    glPolygonMode(GL_FRONT_AND_BACK, rendering_mode);

    // CAMERA SETUP
    world_to_camera_matrix = glm::lookAt(eye, target, up);

    // change the rotation angle for the lights
    // this allows for the lights to be rotated
    light_rotationY += 0.4f;
    if (light_rotationY > 360.0f) light_rotationY -= 360.0f;

    spotlight_red->m_transformed_position = spotlight_red->m_initial_position;
    spotlight_red->m_transformed_target = spotlight_red->m_initial_target;
    spotlight_blue->m_transformed_position = spotlight_blue->m_initial_position;
    spotlight_blue->m_transformed_target = spotlight_blue->m_initial_target;
    // Uncomment this to rotate light (also need to uncomment the DrawSpotLightSource to get the sphere that rotates the light to rotate as well)
    spotlight_red->m_transformed_position = glm::vec3(glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::vec4(spotlight_red->m_initial_position, 1.0f));
    spotlight_blue->m_transformed_position = glm::vec3(glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::vec4(spotlight_blue->m_initial_position, 1.0f));
    spotlight_red->m_transformed_target = glm::vec3(glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::vec4(spotlight_red->m_initial_target, 1.0f));
    spotlight_blue->m_transformed_target = glm::vec3(glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::vec4(spotlight_blue->m_initial_target, 1.0f));

    SceneGraphDraw();

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

void DrawSpotLightSource(SpotLight* _spotlight)
{
    // draw the light source
    // we draw the light source (for the omni light) by translating a small sphere onto the light's position
    // this is the local transform (S T R) which scales, translates and rotates on the translated orbit
    //glm::mat4x4 obj = glm::translate(glm::vec3(_spotlight->m_initial_position)) * glm::scale(0.2f, 0.2f, 0.2f);
    // rotate the sphere so that it follows the light
    glm::mat4x4 obj = glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::translate(glm::vec3(_spotlight->m_initial_position)) * glm::scale(0.4f, 0.4f, 0.4f);

    // we also need to build the world transform (since we can rotate the scene with the mouse or move the world with the arrow keys)
    glm::mat4x4 wld = glm::translate(world_translate) * glm::rotate(world_rotate_x, 0.0f, 1.0f, 0.0f);
    obj = wld * obj;

    // draw the light source
    DrawLightSource(lightSourceMesh, obj, _spotlight->m_color);
}

void SceneGraphDraw()
{
    if (root == nullptr) return;

    // USE SCENE GRAPH
    root->SetViewMat(world_to_camera_matrix);
    root->SetProjectionMat(perspective_projection_matrix);

    // world transformations
    world_transform->SetTranslation(world_translate.x, world_translate.y, world_translate.z);
    world_transform->SetRotation(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // the logic is behind rendering the scene using different lights is:
    // render the scene once for each light source and add them together using blending
    // where the glBlendFunc is set to additive. This is because lights are additive in nature.
    // Each time you render the scene, is called a rendering pass.
    // The ambient light is added on a separate pass (using an ambient light shader)
    // Steps:
    // 1) before the first pass, clear depth and color buffers
    // 2) render the scene (common practice here is to render the ambient light)
    // at the end of this pass, the color buffer contains the colors of the ambient light for the current frame
    // 3) enable additive blending (to add the lights)
    // 4) render the scene using a different light source each time
    // 5) once you are done, disable blending

    // 1
    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // 2 render the scene with the ambient light shader
    root->Draw(1);
    // for the purposes of this tutorial, also draw the light sources
    DrawSpotLightSource(spotlight_red);
    DrawSpotLightSource(spotlight_blue);
    // 3
    // enable blending
    glEnable(GL_BLEND);
    // set blending to add the current color of the fragment with the fragment color that is already there
    glBlendFunc(GL_ONE, GL_ONE);
    // 4
    // draw the lights (this can be in a for loop)
    // render the scene using the spotlight shader
    // red spotlight
    root->SetActiveSpotlight(spotlight_red);
    glm::mat4x4 wld = glm::translate(world_translate) * glm::rotate(world_rotate_x, 0.0f, 1.0f, 0.0f);
    spotlight_red->m_transformed_position = glm::vec3(wld * glm::vec4(spotlight_red->m_transformed_position, 1.0f));
    spotlight_red->m_transformed_target = glm::vec3(wld * glm::vec4(spotlight_red->m_transformed_target, 1.0f));
    root->Draw(0);
    // blue spotlight
    root->SetActiveSpotlight(spotlight_blue);
    spotlight_blue->m_transformed_position = glm::vec3(wld * glm::vec4(spotlight_blue->m_transformed_position, 1.0f));
    spotlight_red->m_transformed_target = glm::vec3(wld * glm::vec4(spotlight_blue->m_transformed_target, 1.0f));
    root->Draw(0);
    // for the purposes of this tutorial, also draw the light sources
    DrawSpotLightSource(spotlight_red);
    DrawSpotLightSource(spotlight_blue);
    // 5
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

// This is the rendering code (taken from the Draw in the GeometryNode) that renders a mesh
// loaded from an OBJ file
// This is used to draw the light sources using an emissive color parameter
void DrawLightSource(OGLMesh* mesh, glm::mat4x4& object_to_world_transform, glm::vec3& light_emissive_color)
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

    // pass the emissive color
    glUniform4f(basic_geometry_shader->uniform_emissive_color, light_emissive_color.x, light_emissive_color.y, light_emissive_color.z, 1.0f);

    // loop through all the elements
    for (GLint i=0; i < mesh->num_elements; i++)
    {
        if (mesh->elements[i].triangles==0)
            continue;

        // Material and texture goes here.
        int mtrIdx = mesh->elements[i].material_index;
        OBJMaterial& cur_material = *materials[mtrIdx];

        // use the material color
        glUniform4f(basic_geometry_shader->uniform_material_color, cur_material.m_diffuse[0], cur_material.m_diffuse[1], cur_material.m_diffuse[2], cur_material.m_opacity);

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