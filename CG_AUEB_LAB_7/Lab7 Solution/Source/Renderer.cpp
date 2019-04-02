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

// window settings
float current_width;
float current_height;

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
// for scene 3
TransformNode* knossos_transform;
GeometryNode* knossos_geom;

// Shaders
// basic geometry shader
BasicGeometryShader* basic_geometry_shader;
// spotlight shader
SpotLightShader* spotlight_shader;
// shadow map shader
ShadowMapShader* shadowmap_shader;

// Lights
SpotLight* spotlight;

// light parameters (for animating the light)
float light_rotationY;

// frame buffer object
GLuint shadow_fbo;
GLuint shadow_tex;
float shadow_dimensions;

// forward declarations
bool CreateShadowFBO();
void DrawSceneToShadowFBO();
bool CreateShaders();
bool LoadObjModels();
void DrawOGLMesh(OGLMesh* mesh, glm::mat4x4& object_to_world_transform);
void SceneGraphExampleInit();
void SceneGraphExample2Init();
void SceneGraphExample3Init();
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
    spotlight = new SpotLight();
    spotlight->m_name = "spotlight";

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

    // Initialize the FBO
    CreateShadowFBO();

    // Enable Scene Graph
    /*
    SceneGraphExampleInit();
    */

    /*
    SceneGraphExample2Init();
    */

    SceneGraphExample3Init();

    // check if we have generated any OpenGL errors
    glError();

    return true;
}

bool CreateShadowFBO()
{
    // set the shadow dimensions
    shadow_dimensions = 1024.0f;

    // Create a framebuffer object (FBO)
    // When bound, rendering will happen on the FBO's attached textures
    glGenFramebuffers(1, &shadow_fbo);
    // set the fbo active
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);

    // since we need to render only depth maps, we tell OpenGL that we do not need
    // to write to any color buffer
    glDrawBuffer(GL_NONE);
    // similarly, we do not need to read from any color buffer
    glReadBuffer(GL_NONE);

    // generate a texture to write to
    glGenTextures(1, &shadow_tex);
    // set the texture active
    glBindTexture(GL_TEXTURE_2D, shadow_tex);

    PrintToOutputWindow("%s, Generated Depth Texture: %i.", spotlight->m_name.c_str(), shadow_tex);

    // allocate space for a depth texture
    // each value will be stored in a single channel 32-bit float value
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadow_dimensions, shadow_dimensions, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // we set linear interpolation for the filtering and clamp to edge for wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // FBO's do not have any default buffers to write to
    // instead, we need to set a number of images attached to them which will specify where to write
    // information such as color and depth
    // The common approach is to attach one or more color textures and one depth texture
    // For shadow mapping, only a depth texture needs to be attached (Which was created above)
    // the depth texture is attached to the depth attachment point of the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_tex, 0);

    // check for any FBO errors (similar to glError)
    // this checks for example, if the attachments that were set using glFramebufferTexture are valid
    if (checkFrameBufferError("(Depth) Incomplete SpotLight fbo")) return false;

    // set the texture id to the spotlight object
    // this is later used in the geometry node to pass it as a texture to the spotlight shader
    spotlight->m_shadow_map_texture_id = shadow_tex;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    spotlight->m_color = glm::vec3(1.0f, 0.57f, 0.16f);
    spotlight->m_initial_target = glm::vec3(0.0f, 0.0f, 0.0f);
    spotlight->m_initial_position = glm::vec3(5.0f, 20.0f, 0.0f);
    spotlight->m_near_range = 1.0f;
    spotlight->m_far_range = 100.0f;
    spotlight->m_aperture = 60.0f;

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
    root->SetShadowMapShader(shadowmap_shader);

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
    eye = glm::vec3(80.0f, 60.0f, 100.0f);

    // light settings
    spotlight->m_color = glm::vec3(1.0f, 0.57f, 0.16f);
    spotlight->m_initial_target = glm::vec3(0.0f, 0.0f, 0.0f);
    spotlight->m_initial_position = glm::vec3(30.0f, 60.0f, 0.0f);
    spotlight->m_near_range = 1.0f;
    spotlight->m_far_range = 100.0f;
    spotlight->m_aperture = 60.0f;

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
    root->SetShadowMapShader(shadowmap_shader);

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

void SceneGraphExample3Init()
{
    // Scene graph example 3
    // background settings
    // Set the RGBA values to clear the color buffer (using glClear later on)
    glClearColor(0.03f, 0.2f, 0.22f, 1.0f); // for spot light

    // eye settings
    eye = glm::vec3(0.0f, 100.0f, 170.0f);

    // light settings
    spotlight->m_color = glm::vec3(1.0f, 0.57f, 0.16f);
    spotlight->m_initial_target = glm::vec3(0.0f, 0.0f, 0.0f);
    spotlight->m_initial_position = glm::vec3(60.0f, 30.0f, 0.0f);
    spotlight->m_near_range = 1.0f;
    spotlight->m_far_range = 100.0f;
    spotlight->m_aperture = 60.0f;

    // scene settings
    root = new Root();
    world_transform = new TransformNode("world_transform");
    knossos_transform = new TransformNode("knossos_transform");
    knossos_geom = new GeometryNode("knossos_geom", knossosMesh);

    // construct the render tree
    root->AddChild(world_transform);
    world_transform->AddChild(knossos_transform);
    knossos_transform->AddChild(knossos_geom);

    // initialize root (sets the parent->child relationships in the tree)
    root->Init();

    // since we are using more than one shaders, we are passing them to the root node
    // so they can be accessible from the geometry node during the Draw call
    // this is because each shader requires different uniform variables
    root->SetSpotlightShader(spotlight_shader);
    root->SetShadowMapShader(shadowmap_shader);
}

bool LoadObjModels()
{
    OBJLoader* objLoader = new OBJLoader();

    // for all scenes
    spherewhiteMesh = objLoader->loadMesh("sphere_white.obj", "..\\..\\Data\\Other", true);
    groundwhiteMesh = objLoader->loadMesh("plane_white.obj", "..\\..\\Data\\Other", true);

    // for scene 1
    sphereMapMesh = objLoader->loadMesh("sphere_map.obj", "..\\..\\Data\\Other", true);
    sphereEarthMesh = objLoader->loadMesh("sphere_earth.obj", "..\\..\\Data\\Other", true);

    // for scene 2
    skeletonGroundMesh = objLoader->loadMesh("terrain.obj", "..\\..\\Data\\Other", true);
    treasureMesh = objLoader->loadMesh("treasure.obj", "..\\..\\Data\\Pirates", true);
    skeletonMesh = objLoader->loadMesh("skeleton.obj", "..\\..\\Data\\Pirates", true);

    // for scene 3
    knossosMesh = objLoader->loadMesh("knossos.obj", "..\\..\\Data\\Knossos", true);

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

    // these are for the samplers
    basic_geometry_shader->uniform_sampler_diffuse = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_sampler_diffuse");
    basic_geometry_shader->uniform_has_sampler_diffuse = glGetUniformLocation(basic_geometry_shader->program_id, "uniform_has_sampler_diffuse");

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
    spotlight_shader->uniform_color = glGetUniformLocation(spotlight_shader->program_id, "uniform_material_color");
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

    // these are for the shadow calculations
    spotlight_shader->uniform_sampler_shadow_map = glGetUniformLocation(spotlight_shader->program_id, "uniform_sampler_shadow_map");
    spotlight_shader->uniform_view_inverse = glGetUniformLocation(spotlight_shader->program_id, "uniform_view_inverse");
    spotlight_shader->uniform_light_view_projection = glGetUniformLocation(spotlight_shader->program_id, "uniform_light_view_projection");

    // Shadow Map shader
    // This is used for rendering the geometry to the shadow map depth buffer
    shadowmap_shader = new ShadowMapShader();
    shadowmap_shader->shader = new ShaderGLSL("BasicShadow");
    // compile
    shader_loaded = shadowmap_shader->shader->LoadAndCompile();
    if (!shader_loaded) return false;
    // get the program id
    shadowmap_shader->program_id = shadowmap_shader->shader->GetProgram();
    // check for uniforms
    shadowmap_shader->uniform_mvp = glGetUniformLocation(shadowmap_shader->program_id, "uniform_mvp");

    // all shaders loaded OK
    return true;
}

// Render function. Every time our window has to be drawn, this is called.
void Render(void)
{
    if (root == nullptr) return;

    // Set the rendering mode
    glPolygonMode(GL_FRONT_AND_BACK, rendering_mode);

    // CAMERA SETUP
    world_to_camera_matrix = glm::lookAt(eye, target, up);

    // change the rotation angle for the lights
    // this allows for the lights to be rotated
    light_rotationY += 0.4f;
    if (light_rotationY > 360.0f) light_rotationY -= 360.0f;

    spotlight->m_transformed_position = spotlight->m_initial_position;
    spotlight->m_transformed_target = spotlight->m_initial_target;
    // Uncomment this to rotate light (also need to uncomment the DrawSpotLightSource to get the sphere that rotates the light to rotate as well)
    spotlight->m_transformed_position = glm::vec3(glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * glm::vec4(spotlight->m_initial_position, 1.0f));

    // Write to Shadow Map FBO
    DrawSceneToShadowFBO();

    // now, render the scene as usual
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

void DrawSceneToShadowFBO()
{
    // switch the rendering to happen on the FBO rather than the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_fbo);
    // clear the depth color for this framebuffer
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Create the worldviewprojection matrix for the light source
    // Again, since a light node is missing, the world transformation needs to be applied
    glm::mat4x4 wld = glm::translate(world_translate) * glm::rotate(world_rotate_x, 0.0f, 1.0f, 0.0f);
    spotlight->m_transformed_position = glm::vec3(wld * glm::vec4(spotlight->m_transformed_position, 1.0f));
    spotlight->m_transformed_target = glm::vec3(wld * glm::vec4(spotlight->m_transformed_target, 1.0f));

    // Create the light's world to view space matrix
    // we need to build a "camera" as viewed from the light
    // so we need an up vector, a target and a light "eye" position
    // create the direction vector
    glm::vec3 light_direction = glm::normalize(spotlight->m_transformed_target - spotlight->m_transformed_position);

    // this check is simply a sanity check for the internal cross product in glm::lookAt
    // just in case the light direction vector is 0,1,0
    // if it is, the up vector is set to 0,0,1
    glm::vec3 up;
    if (fabs(light_direction.z) < 0.001 && fabs(light_direction.x) < 0.001)
        up = glm::vec3(0,0,1);
    else
        up = glm::vec3(0,1,0);

    // construct the light view matrix that transforms world space to light view space (WCS -> LCS)
    // LCS is the view space of the light, similar to ECS which is the view space for the camera
    glm::mat4x4 world_to_light_view_matrix = glm::lookAt(spotlight->m_transformed_position, spotlight->m_transformed_target, up);

    //float h = spotlight->m_near_range *glm::tan(glm::radians(spotlight->m_aperture * 0.5f));
    //glm::mat4x4 light_projection_matrix = glm::frustum(-h, h, -h, h, spotlight->m_near_range, spotlight->m_far_range);
    // aspect ratio is 1 since both width and height are the same (dimensions of the texture)
    glm::mat4x4 light_projection_matrix = glm::perspective(90.0f, 1.0f, spotlight->m_near_range, spotlight->m_far_range);

    // also we need to set a new viewport
    // this viewport has the dimensions of the target depth texture (the shadow texture)
    glViewport(0.0f, 0.0f, shadow_dimensions, shadow_dimensions);

    // now draw the scene as usual

    // USE SCENE GRAPH
    root->SetLightViewMat(world_to_light_view_matrix);
    root->SetLightProjectionMat(light_projection_matrix);

    // also set the world transformations here since they will be retrieved as part of the M matrix in the GeometryNode
    world_transform->SetTranslation(world_translate.x, world_translate.y, world_translate.z);
    world_transform->SetRotation(world_rotate_x, 0.0f, 1.0f, 0.0f);

    // Draw the scene by traversing the scene graph
    // 0 renders the geometry using the spotlight shader
    // 1 renders the geometry to the shadow map
    root->Draw(1);

    // unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DrawSpotLightSource()
{
    // draw the light source
    // we draw the light source (for the omni light) by translating a small sphere onto the light's position
    // this is the local transform (S T R) which scales, translates and rotates on the translated orbit
    glm::mat4x4 obj = glm::translate(glm::vec3(spotlight->m_initial_position)) * glm::scale(0.2f, 0.2f, 0.2f);
    // rotate the sphere so that it follows the light
    obj = glm::rotate(light_rotationY, 0.0f, 1.0f, 0.0f) * obj;

    // we also need to build the world transform (since we can rotate the scene with the mouse or move the world with the arrow keys)
    glm::mat4x4 wld = glm::translate(world_translate) * glm::rotate(world_rotate_x, 0.0f, 1.0f, 0.0f);
    obj = wld * obj;

    // draw the mesh
    DrawOGLMesh(spherewhiteMesh, obj);
}

void SceneGraphDraw()
{
    if (root == nullptr) return;

    // right before rendering we need to set the target framebuffer to
    // the default framebuffer
    // this is because we used an FBO to switch the rendering to another location
    // set the back buffer as the target framebuffer
    glDrawBuffer(GL_BACK);

    // Clear depth and color buffers.
    // We can clear both using an | operand to create the buffer-clear mask.
    // glClear is called here as well because we are working on the default framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the viewport back to the actual window dimensions (it was changed by the FBO rendering code)
    glViewport(0.0f, 0.0f, current_width, current_height);

    // draw the light source for the spot light
    DrawSpotLightSource();

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
    root->SetActiveSpotlight(spotlight);

    // Draw the scene by traversing the scene graph
    // 0 renders the geometry using the spotlight shader
    // 1 renders the geometry to the shadow map
    root->Draw(0);
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

    // keep these values globally to reset the viewport after rendering to an FBO
    current_width = width;
    current_height = height;
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