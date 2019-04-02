﻿//----------------------------------------------------//
//                                                    //
// File: Renderer.cpp                                 //
// Initialization, rendering and input handling       //
// happens here                                       //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided for the tutorials as part //
// of the BSc course of Computer Graphics at the      //
// Athens University of Economics and Business (AUEB) //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

#include "HelpLib.h"    // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.

#include "ShaderGLSL.h" // - Header file for GLSL objects
#include "Renderer.h"   // - Header file for our OpenGL functions

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

// object rotations
float planet1_angle = 0.0f;
float planet2_angle = 0.0f;

// vertex array objects
GLuint vao_sphere;
GLint vao_sphere_indices;

// BasicGeometry
ShaderGLSL* bgs_glsl = nullptr;
// the program id of the shader
// this id identifies which shader we are referring to
GLint bgs_program_id;
// the id of a uniform variable for the above shader
GLint bgs_uniform_mvp;
// the id of a uniform variable for the above shader
GLint bgs_uniform_color;

// BasicLight
ShaderGLSL* bl_glsl = nullptr;
// the program id of the shader
// this id identifies which shader we are referring to
GLint bl_program_id;
// the id of a uniform variable for the above shader
GLint bl_uniform_mvp;
// the id of a uniform variable for the above shader
GLint bl_uniform_normal_matrix;
// the id of a uniform variable for the above shader
GLint bl_uniform_color;

// Rendering mode
int rendering_mode = GL_FILL;

// forward declarations
void BuildSphereVAO2(float, int, int);
void DrawPlanets(glm::mat4x4& parent_transform);
void DrawPlanet1(glm::mat4x4& parent_transform);
void DrawPlanet2(glm::mat4x4& parent_transform);

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
    eye = glm::vec3(0.0f, 0.0f, 70.0f);
    target = glm::vec3(0.0f, 0.0f, 0.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    // -------------------------------------------SHADER CREATION---------------------------------------//
    // create a basic GLSL shader for rendering simple primitives with different colors per vertex
    // create a GLSL object. The name passed is the actual name of the shader
    bgs_glsl = new ShaderGLSL("BasicGeometry");
    // load the shader into memory and compile it
    bool shader_loaded = bgs_glsl->LoadAndCompile();
    // if the shader has not been loaded correctly, the VS Output window will contain the errors produced
    // and the application will exit
    if (!shader_loaded) return false;
    // once compiled, OpenGL creates a program ID, used to identify the shader. Store this for later use during rendering.
    bgs_program_id = bgs_glsl->GetProgram();
    // Each shader, can contain a number of variables we pass from the C++ side. Again, we need an ID for this for later use during rendering.
    // glGetUniformLocation returns the id of a uniform variable for a specific shader. The uniform variable is identified by its name in the shader files (vert or frag)
    // So, the command below checks the BasicGeometry vertex and fragment shaders for a uniform variable named uniform_mvp
    // If this does not exists, glGetUniformLocation will return -1
    bgs_uniform_mvp = glGetUniformLocation(bgs_program_id, "uniform_mvp");
    bgs_uniform_color = glGetUniformLocation(bgs_program_id, "uniform_color");

    // create a basic GLSL shader for rendering simple primitives with different colors per vertex
    // create a GLSL object. The name passed is the actual name of the shader
    bl_glsl = new ShaderGLSL("BasicLighting");
    // load the shader into memory and compile it
    shader_loaded = bl_glsl->LoadAndCompile();
    // if the shader has not been loaded correctly, the VS Output window will contain the errors produced
    // and the application will exit
    if (!shader_loaded) return false;
    // once compiled, OpenGL creates a program ID, used to identify the shader. Store this for later use during rendering.
    bl_program_id = bl_glsl->GetProgram();
    // Each shader, can contain a number of variables we pass from the C++ side. Again, we need an ID for this for later use during rendering.
    // glGetUniformLocation returns the id of a uniform variable for a specific shader. The uniform variable is identified by its name in the shader files (vert or frag)
    // So, the command below checks the BasicGeometry vertex and fragment shaders for a uniform variable named uniform_mvp
    // If this does not exists, glGetUniformLocation will return -1
    bl_uniform_mvp = glGetUniformLocation(bl_program_id, "uniform_mvp");
    bl_uniform_normal_matrix = glGetUniformLocation(bl_program_id, "uniform_normal_matrix");
    bl_uniform_color = glGetUniformLocation(bl_program_id, "uniform_color");

    // -------------------------------------------------------------------------------------------------//

    // build the VAO
    BuildSphereVAO2(1.0f, 10, 10);

    // Enable the depth buffer
    // Default is disabled
    glDisable(GL_DEPTH_TEST);

    // Initial value is GL_LESS
    glDepthFunc(GL_LEQUAL);

    // Set the RGBA values to clear the color buffer (using glClear later on)
    glClearColor(0.2f, 0.2f, 0.25f, 1.0f);

    // Set the value to clear the depth buffer to value 1.0 (all far) (using glClear later on)
    glClearDepth(1.0f);

    // check if we have generated any OpenGL errors
    glError();

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
    // set world-space to eye-space transformations (WCS->ECS)
    // Note: eye-space is also referred as camera-space or view-space
    // simply put, position the objects relative to the camera
    // this is accomplished by using GLM's lookat function
    // It takes as parameters the camera position, the camera's target and an up vector (to construct internally a camera coordinate system!)
    // BASIC CAMERA SYSTEM (we only change the eye and target coordinates)
    world_to_camera_matrix = glm::lookAt(eye, target, up);

    // apply WORLD transformations for ALL objects
    // create the identity matrix (does nothing :))
    glm::mat4x4 world_matrix = glm::mat4x4(1);
    world_matrix = glm::rotate(world_rotate_x, 1.0f, 0.0f, 0.0f);

    // create a parent transform
    // this will hold the up-to-now world transformation and allow for hierarchical transformations
    // for example: the wheels of a moving car should rotate on their own local frame
    // but their location is always dependent on the current location of the car
    glm::mat4x4 parent_transform = world_matrix;

    // Enable Face Culling
    // Default state is disabled
    glEnable(GL_CULL_FACE);

    // Enable the depth buffer
    glEnable(GL_DEPTH_TEST);

    DrawPlanets(parent_transform);

    // Remember that we are using double-buffering, all the drawing we just did
    // took place in the "hidden" back-buffer. Calling glutSwapBuffers makes the
    // back buffer "visible".
    glutSwapBuffers();

    glError();
}

// 2 planets on a planetary system
// Planet 1 at the center which rotates around itself,
// Planet 2 at the top right of Planet 1 which also rotates around itself
// Since we do not use materials and textures yet, this is better viewed in wireframe mode
void DrawPlanets(glm::mat4x4& parent_transform)
{
    // random rotation offsets
    // this check is so that each time we exceed 360 degrees we go back to 0 (avoids overflow)
    if (planet1_angle > 360.0f) planet1_angle -= 360.0f;
    if (planet2_angle > 360.0f) planet2_angle -= 360.0f;

    planet1_angle += 0.4f;
    planet2_angle += 0.4f;

    DrawPlanet1(parent_transform);
}

void DrawPlanet1(glm::mat4x4& parent_transform)
{
    // PLANET 1
    // planet 1 rotates around itself
    // NOTE: initially the sphere is at its origin
    // Perform a simple rotation on the Y axis
    glm::mat4x4 rotation_matrix = glm::rotate(planet1_angle, 0.0f, 1.0f, 0.0f);
    // The sphere is of radius 1, lets get make it larger.
    glm::mat4x4 scale_matrix = glm::scale(3.0f, 3.0f, 3.0f);
    // Create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // draw sphere with the current MVP matrix
    // Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bl_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bl_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // pass the color of sphere 1
    glUniform4f(bl_uniform_color, 1.0f, 0.0f, 0.0f, 1.0f);

    // create the normal matrix
    glm::mat4x4 normal_matrix = glm::transpose(glm::inverse(world_to_camera_matrix * object_to_world_matrix));
    // pass the normal matrix as uniform
    glUniformMatrix4fv(bl_uniform_normal_matrix, 1, false, &normal_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_sphere);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_TRIANGLES, 0, vao_sphere_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);

    // now, based on this planet, place a new planet on the right
    // to do this, we need to pass the global transformations that will affect the local transformations of the new planet
    // since the size of one planet does not affect the size of the other, we do not pass the scale
    glm::mat4x4 planet2_parent_transform = parent_transform * rotation_matrix;
    DrawPlanet2(planet2_parent_transform);
}

void DrawPlanet2(glm::mat4x4& parent_transform)
{
    // PLANET 2
    // planet 2 rotates around itself and around planet 1
    // NOTE: initially the sphere is at its origin
    // Since we want the sphere to rotate around itself we perform the rotation first
    // and then we translate it wherever we want!
    glm::mat4x4 translation_matrix = glm::translate(10.0f, 2.0f, 0.0f);
    glm::mat4x4 rotation_matrix = glm::rotate(planet2_angle, 0.0f, 1.0f, 0.0f);
    glm::mat4x4 scale_matrix = glm::scale(1.2f, 1.2f, 1.2f);
    // final OCS->WCS matrix
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // draw sphere with the current MVP matrix
    // Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bl_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bl_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);
    // pass the color of sphere 2
    glUniform4f(bl_uniform_color, 0.0f, 0.0f, 1.0f, 1.0f);

    // create the normal matrix
    glm::mat4x4 normal_matrix = glm::transpose(glm::inverse(world_to_camera_matrix * object_to_world_matrix));
    // pass the normal matrix as uniform
    glUniformMatrix4fv(bl_uniform_normal_matrix, 1, false, &normal_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_sphere);
    // Draw!!
    glDrawArrays(GL_TRIANGLES, 0, vao_sphere_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Release all memory allocated by pointers using new
void ReleaseGLUT()
{
    SAFE_DELETE(bgs_glsl);
    SAFE_DELETE(bl_glsl);
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
        break;
    case GLUT_KEY_PAGE_DOWN:
        break;
    case GLUT_KEY_UP:
        break;
    case GLUT_KEY_DOWN:
        break;
    case GLUT_KEY_LEFT:
        break;
    case GLUT_KEY_RIGHT:
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
    float far_field_value = 100.0f;

    // --------------------------------PERSPECTIVE PROJECTION-------------------------------------------//
    // each time we resize the window, a new projection matrix must be defined
    // use GLM to create a perspective projection matrix (good for 3D rendering)
    // vertical field of view angle
    float vertical_FOV = 30.0f;

    // use GLM to create a perspective projection matrix
    perspective_projection_matrix = glm::perspective(vertical_FOV, aspect_ratio, near_field_value, far_field_value);

    // -------------------------------------------------------------------------------------------------//
}

struct SphereVertexStruct
{
    glm::vec3 position;
    glm::vec3 normal;
};

void BuildSphereVAO2(float sphere_radius, int longitude_steps, int latitude_steps)
{
    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_sphere);
    // Set the VAO active
    glBindVertexArray(vao_sphere);

    // add vertex data
    std::vector<SphereVertexStruct> sphere_data;

    if (longitude_steps < 2) longitude_steps = 2;
    if (latitude_steps < 4) latitude_steps = 4;

    latitude_steps = 40;
    longitude_steps = 20;

    float phi_step = 2 * glm::pi<float>() / float(latitude_steps);
    float theta_step = glm::pi<float>() / float(longitude_steps);
    float phi = 0;
    float theta = 0;

    glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

    for (int j = 0; j < longitude_steps; j++)
    {
        // temp variables to avoid calculating trigonometric values many times
        float costheta = glm::cos(theta);
        float sintheta = glm::sin(theta);
        float costheta_plus_step = glm::cos(theta + theta_step);
        float sintheta_plus_step = glm::sin(theta + theta_step);

        phi = 0;
        for (int i = 0; i < latitude_steps; i++)
        {
            // temp variables to avoid calculating trigonometric values many times
            float cosphi = glm::cos(phi);
            float sinphi = glm::sin(phi);
            float cosphi_plus_step = glm::cos(phi + phi_step);
            float sinphi_plus_step = glm::sin(phi + phi_step);

            SphereVertexStruct top_left;
            top_left.position.x = sphere_radius * sintheta * cosphi;
            top_left.position.z = sphere_radius * sintheta * sinphi;
            top_left.position.y = sphere_radius * costheta;
            top_left.normal = glm::normalize(-top_left.position);

            SphereVertexStruct top_right;
            top_right.position.x = sphere_radius * sintheta * cosphi_plus_step;
            top_right.position.z = sphere_radius * sintheta * sinphi_plus_step;
            top_right.position.y = sphere_radius * costheta;
            top_right.normal = glm::normalize(-top_right.position);

            SphereVertexStruct bottom_left;
            bottom_left.position.x = sphere_radius * sintheta_plus_step * cosphi;
            bottom_left.position.z = sphere_radius * sintheta_plus_step * sinphi;
            bottom_left.position.y = sphere_radius * costheta_plus_step;
            bottom_left.normal = glm::normalize(-bottom_left.position);

            SphereVertexStruct bottom_right;
            bottom_right.position.x = sphere_radius * sintheta_plus_step * cosphi_plus_step;
            bottom_right.position.z = sphere_radius * sintheta_plus_step * sinphi_plus_step;
            bottom_right.position.y = sphere_radius * costheta_plus_step;
            bottom_right.normal = glm::normalize(-bottom_right.position);

            sphere_data.push_back(bottom_left);
            sphere_data.push_back(bottom_right);
            sphere_data.push_back(top_right);
            sphere_data.push_back(top_right);
            sphere_data.push_back(top_left);
            sphere_data.push_back(bottom_left);

            //PrintToOutputWindow("Phi: %.2f -> %.2f, Theta: %.2f -> %.2f", glm::degrees(phi), glm::degrees(phi + phi_step), glm::degrees(theta), glm::degrees(theta + theta_step));
            phi += phi_step;
        }
        theta += theta_step;
    }

    GLsizei stride = sizeof(SphereVertexStruct);

    // Total size of vertex buffer is: num of vertices of 28 bytes each
    int total_vertex_byte_size = sphere_data.size() * stride;

    // we do not use any indices here, so the total number of indices, is the number of vertices in the vertex buffer
    vao_sphere_indices = sphere_data.size();

    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_sphere);
    // Set the VAO active
    glBindVertexArray(vao_sphere);

    GLuint vbo_sphere = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_sphere);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere);
    glBufferData(GL_ARRAY_BUFFER, total_vertex_byte_size, &sphere_data[0], GL_STATIC_DRAW);

    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}