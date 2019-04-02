//----------------------------------------------------//
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
GLuint vao_triangle;
GLint vao_triangle_num_of_indices;
GLuint vao_lines;
GLuint vao_lines_num_vertices;
GLuint vao_quads;
GLint vao_quads_num_of_indices;
GLint vao_quads_num_of_indices2;
GLuint vao_triangle_fan;
GLint vao_triangle_fan_num_of_indices;
GLuint vao_sphere;
GLint vao_sphere_indices;

// shader objects
ShaderGLSL* bgs_glsl = nullptr;
// the program id of the shader
// this id identifies which shader we are referring to
GLint bgs_program_id;
// the id of a uniform variable for the above shader
GLint bgs_uniform_mvp;
// the id of a uniform variable for the above shader
GLint bgs_uniform_color;

// Rendering mode
int rendering_mode = GL_FILL;

// forward declarations
void BuildTriangleVAO();
void BuildLinesVAO();
void BuildQuadsVAO();
void BuildTriangleFanVAO();
void BuildSphereVAO(float, int, int);
void DrawPrimitives(glm::mat4x4& parent_transform);
void DrawTriangle(glm::mat4x4& parent_transform);
void DrawQuadsAsPoints(glm::mat4x4& parent_transform);
void DrawLines(glm::mat4x4& parent_transform);
void DrawLineStrips(glm::mat4x4& parent_transform);
void DrawQuadAsTriangleStrips(glm::mat4x4& parent_transform);
void DrawQuadsAsTriangleStripsWithIndexBuffer1(glm::mat4x4& parent_transform);
void DrawQuadsAsTriangleStripsWithIndexBuffer2(glm::mat4x4& parent_transform);
void DrawTriangleFan(glm::mat4x4& parent_transform);
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
    vao_triangle = 0;
    vao_lines = 0;
    vao_quads = 0;
    vao_triangle_fan = 0;
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
    // -------------------------------------------------------------------------------------------------//

    // build the data structures for a triangle and a quad
    BuildTriangleVAO();
    BuildLinesVAO();
    BuildQuadsVAO();
    BuildTriangleFanVAO();
    BuildSphereVAO(1.0f, 20, 40);

    // Enable the depth buffer
    // Default is disabled
    glDisable(GL_DEPTH_TEST);

    // glDepthFunc tells OpenGL to accept fragments whose depth is less or equal than the current depth value
    // Note that 0.0 denotes the near field and 1.0 denotes the far field
    /* Possible values
    GL_NEVER        Never passes.
    GL_LESS            Passes if the incoming depth value is less than the stored depth value.
    GL_EQUAL        Passes if the incoming depth value is equal to the stored depth value.
    GL_LEQUAL        Passes if the incoming depth value is less than or equal to the stored depth value.
    GL_GREATER        Passes if the incoming depth value is greater than the stored depth value.
    GL_NOTEQUAL        Passes if the incoming depth value is not equal to the stored depth value.
    GL_GEQUAL        Passes if the incoming depth value is greater than or equal to the stored depth value.
    GL_ALWAYS        Always passes.
    */
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

    /*
    // Enable Face Culling
    // Default state is disabled
    glEnable(GL_CULL_FACE);
    // Tell OpenGL to cull back faces
    // Possible values are GL_FRONT, GL_BACK, and GL_FRONT_AND_BACK
    // Default is GL_BACK
    // GL_FRONT_AND_BACK culls everything but lines and points
    glCullFace(GL_BACK);
    // Tell OpenGL that the front faces are the ones in counter-clockwise order
    // Possible values are GL_CW and GL_CCW
    // Default is GL_CCW
    glFrontFace(GL_CCW);
    */

    // DrawPrimitives(parent_transform);

    /*
    // Enable depth testing
    // Default is disabled
    glEnable(GL_DEPTH_TEST);
    */

    // DrawPlanets(parent_transform);

    // Remember that we are using double-buffering, all the drawing we just did
    // took place in the "hidden" back-buffer. Calling glutSwapBuffers makes the
    // back buffer "visible".
    glutSwapBuffers();

    glError();
}

// This functions demonstrates rendering several primitives using the the VAOs
// created during init: vao_triangle, vao_lines, vao_quads and vao_triangle_fan
// Each function draws a 2D object from left to right
// The same VAO can be used to render an object using different rendering parameters,
// For example, the quads VAO and be rendered
// as points using GL_POINTS,
// as lines using GL_LINES_STRIP and
// as triangles using GL_TRIANGLES_STRIP
void DrawPrimitives(glm::mat4x4& parent_transform)
{
    // draw the lines VAO using GL_LINES
    DrawLines(parent_transform);

    // draw the triangles VAO using GL_TRIANGLES
    DrawTriangle(parent_transform);

    // draw the quads VAO using GL_POINTS
    DrawQuadsAsPoints(parent_transform);

    // draw the quads VAO using GL_LINES_STRIP
    DrawLineStrips(parent_transform);

    // draw the quads VAO using GL_TRIANGLES_STRIP
    DrawQuadAsTriangleStrips(parent_transform);

    // draw the quads VAO using GL_TRIANGLES and the index buffer
    DrawQuadsAsTriangleStripsWithIndexBuffer1(parent_transform);

    // draw the quads VAO using GL_TRIANGLES and the index buffer
    DrawQuadsAsTriangleStripsWithIndexBuffer2(parent_transform);

    // draw the triangle fan VAO using GL_TRIANGLE_FAN
    DrawTriangleFan(parent_transform);
}

// Draw quads using ling strips
void DrawLines(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(-10.0f, 0.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_lines);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_LINES, 0, vao_lines_num_vertices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw a triangle
void DrawTriangle(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(-3.0f, 0.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_triangle);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_TRIANGLES, 0, vao_triangle_num_of_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw quads using points
void DrawQuadsAsPoints(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(4.0f, 6.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // set point size (default is 1.0)
    glPointSize(2.0f);

    // Bind the vertex array objects
    glBindVertexArray(vao_quads);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_POINTS, 0, vao_quads_num_of_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw quads using line strips
void DrawLineStrips(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(4.0f, 3.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_quads);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_LINE_STRIP, 0, vao_quads_num_of_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw quads using triangle strips
void DrawQuadAsTriangleStrips(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(4.0f, 0.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_quads);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vao_quads_num_of_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw quads using index buffer
void DrawQuadsAsTriangleStripsWithIndexBuffer1(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(4.0f, -3.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_quads);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawElements renders the vertex buffer with the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the number of indices in the array
    // Third parameter is the data type of the index array
    // Fourth parameter is an offset to the index buffer
    glDrawElements(GL_TRIANGLES, vao_quads_num_of_indices2, GL_UNSIGNED_INT, 0);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw quads using index buffer
void DrawQuadsAsTriangleStripsWithIndexBuffer2(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(4.0f, -6.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_quads);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawRangeElements renders the vertex buffer with the presence of an index buffer as the glDrawElements but
    // within a given range in the index buffer
    // First parameter is the primitive type
    // Second parameter is the start index in the index array
    // Third parameter is the end index in the index array
    // Fourth parameter is the number of elements to be rendered
    // Fifth parameter the data type of the index buffer
    // Sixth parameter is an offset to the index buffer
    // Here we render the exact same sequence of primitives as in the DrawTriangleStrips and DrawTriangleIndexed1
    // but we do 6 draw calls instead of 1. Each draw call renders a single triangle in the index buffer
    glDrawRangeElements(GL_TRIANGLES, 0, 3, 3, GL_UNSIGNED_INT, 0);
    glDrawRangeElements(GL_TRIANGLES, 3, 6, 3, GL_UNSIGNED_INT, (GLvoid*)(3 * sizeof(unsigned int)));
    glDrawRangeElements(GL_TRIANGLES, 6, 9, 3, GL_UNSIGNED_INT, (GLvoid*)(6 * sizeof(unsigned int)));
    glDrawRangeElements(GL_TRIANGLES, 9, 12, 3, GL_UNSIGNED_INT, (GLvoid*)(9 * sizeof(unsigned int)));
    glDrawRangeElements(GL_TRIANGLES, 12, 15, 3, GL_UNSIGNED_INT, (GLvoid*)(12 * sizeof(unsigned int)));
    glDrawRangeElements(GL_TRIANGLES, 15, 18, 3, GL_UNSIGNED_INT, (GLvoid*)(15 * sizeof(unsigned int)));

    //
    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
}

// Draw a circle using triangle fan
void DrawTriangleFan(glm::mat4x4& parent_transform)
{
    // Set object-space to world-space transformations (OCS ->WCS)
    // rotation matrix
    glm::mat4x4 rotation_matrix = glm::rotate(0.0f, 0.0f, 1.0f, 0.0f);
    // translation matrix
    glm::mat4x4 translation_matrix = glm::translate(12.0f, 0.0f, 0.0f);
    // scale matrix
    glm::mat4x4 scale_matrix = glm::scale(1.0f, 1.0f, 1.0f);

    // Lets create the object-space to world-space matrix
    // World matrix = T * R * S
    // also apply the parent transformation to this object
    glm::mat4x4 object_to_world_matrix = parent_transform * translation_matrix * rotation_matrix * scale_matrix;

    // create the MVP matrix (MVP = P * V * M)
    // OCS->WCS->ECS->CSS
    glm::mat4x4 model_view_projection_matrix = perspective_projection_matrix * world_to_camera_matrix * object_to_world_matrix;

    // Lets start rendering. Set the shader active
    // Then, pass parameters to the shader using glUniform commands
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // Bind the vertex array objects
    glBindVertexArray(vao_triangle_fan);
    // This is the actual draw command
    // Once this is called, all data given by the VAO are being sent down the OpenGL pipeline
    // to be rendered by the active shader program
    // glDrawArrays renders the vertex buffer without the presence of an index buffer
    // First parameter is the primitive type
    // Second parameter is the starting offset of the array
    // Third parameter is the number of vertices (or indices in that case)
    glDrawArrays(GL_TRIANGLE_FAN, 0, vao_triangle_fan_num_of_indices);

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);
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
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);

    // pass the color of sphere 1
    glUniform4f(bgs_uniform_color, 1.0f, 0.0f, 0.0f, 1.0f);

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
    glUseProgram(bgs_program_id);

    // Right before calling glDrawArrays we need to pass the shader uniform parameters
    // Matrices (16-value floats) are passed using glUniformMatrix4fv
    glUniformMatrix4fv(bgs_uniform_mvp, 1, false, &model_view_projection_matrix[0][0]);
    // pass the color of sphere 2
    glUniform4f(bgs_uniform_color, 0.0f, 0.0f, 1.0f, 1.0f);

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
}

// Release all memory allocated by pointers using new
void ReleaseGLUT()
{
    SAFE_DELETE(bgs_glsl);
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

// create a data structure that will hold our vertex data (in the format given)
struct VertexStruct
{
    glm::vec3 position;
    glm::vec3 material_color;
};

// Example of creating a triangle without indices
// this is used during draw to render a triangle using GL_TRIANGLES
void BuildTriangleVAO()
{
    // First, we need to build the data on the CPU side.
    // Then, we pass it onto the GPU

    // to build the vertex data for a triangle we need 3 vertices
    VertexStruct bottom_left;
    VertexStruct bottom_right;
    VertexStruct top;

    // these are the positions in object space for each vertex
    bottom_left.position            = glm::vec3(-1.0f, -1.0f, 0.0f);        // bottom left vertex
    bottom_right.position            = glm::vec3(1.0f, -1.0f, 0.0f);            // bottom right vertex
    top.position                    = glm::vec3(0.0f,  1.0f, 0.0f);            // top vertex

    // these are the colors for each vertex
    bottom_left.material_color        = glm::vec3(1.0f, 0.0f, 0.0f);            // red color for bottom left vertex
    bottom_right.material_color        = glm::vec3(0.0f, 1.0f, 0.0f);            // green color for bottom right vertex
    top.material_color                = glm::vec3(0.0f, 0.0f, 1.0f);            // blue color for top vertex

    // add vertex data in counter-clockwise order
    std::vector<VertexStruct> triangle_data;
    triangle_data.push_back(bottom_left);
    triangle_data.push_back(bottom_right);
    triangle_data.push_back(top);

    // in order to pass the data to the GPU we need to know the total number of bytes our data is
    // in this case position is 3 floats (12 bytes) and color is 3 floats (12 bytes)
    // so each vertex holds 24 bytes.
    // This means that the vertex buffer defines a new vertex in a 24-byte offset. This is called stride.
    // Think of it as a vertex step number
    GLsizei stride = sizeof(VertexStruct);

    // Total size of vertex buffer is: 3 vertices of 24 bytes each
    int total_vertex_byte_size = triangle_data.size() * stride;

    // we do not use any indices here, so the total number of indices, is the number of vertices in the vertex buffer
    vao_triangle_num_of_indices = triangle_data.size();

    // To pass the data to the GPU we do the following:
    // Generate a vertex array object and bind it as active. This will point to the buffer object(s) we will use later on
    // Generate the vertex buffer object(s), set them active and pass the data
    // Since each vertex data might contain more than one attributes, we also need to specify how each vertex data is structured.
    // For example, each vertex here is a struct with one vec3 for the position and one vec3 for the color

    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_triangle);
    // Set the VAO active
    glBindVertexArray(vao_triangle);

    GLuint vbo_triangle = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_triangle);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    // pass the data to the GPU as a sequence of bytes:
    // - The buffer that the vertex attributes come from is always GL_ARRAY_BUFFER
    // - We also pass the total size and a pointer to the first value in the data buffer
    // - GL_STATIC_DRAW is mostly an indication of how the data will be accessed. It does not affect the rendering result.
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glBufferData.xml
    glBufferData(GL_ARRAY_BUFFER, total_vertex_byte_size, &triangle_data[0], GL_STATIC_DRAW);

    // to inform OpenGL how each attribute in the vertex data is structured, we use glVertexAttribPointer
    // The following command means that the first vertex attribute (the position) is accessed by OpenGL in the following way:
    // These should be the same with the layout(location = #)​ in type name syntax in the vertex shader
    // - At index 0
    // - is a vec3
    // - has type float
    // - is NOT normalized (-1,1)
    // - the stride value
    // - the offset in the vertex buffer of the vertex attribute
    // to access it at the vertex shader we use layout(location = 0)​ in vec3 <any_name_here>
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // same here
    // to access this at the vertex shader we use layout(location = 1)​ in vec3 <any_name_here>
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    // once we specify the vertex attributes, we need to set them active
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}

// Example of creating lines without an index buffer
// this is used during draw to render a triangle using GL_LINES
void BuildLinesVAO()
{
    // First, we need to build the data on the CPU side.
    // Then, we pass it onto the GPU

    // to build the vertex data for a quad we need 4 vertices
    VertexStruct v1;
    VertexStruct v2;
    VertexStruct v3;
    VertexStruct v4;
    VertexStruct v5;
    VertexStruct v6;
    VertexStruct v7;
    VertexStruct v8;

    // these are the positions in object space for each vertex
    v1.position                = glm::vec3( -3.0f, 1.0f, 0.0f );        //vertex 1
    v2.position                = glm::vec3( -3.0f, -1.0f, 0.0f );        //vertex 2
    v3.position                = glm::vec3( -1.0f, 1.0f, 0.0f );        //vertex 3
    v4.position                = glm::vec3( -1.0f, -1.0f, 0.0f );        //vertex 4
    v5.position                = glm::vec3( 1.0f, 1.0f, 0.0f );        //vertex 5
    v6.position                = glm::vec3( 1.0f, -1.0f, 0.0f );        //vertex 6
    v7.position                = glm::vec3( 3.0f, 1.0f, 0.0f );        //vertex 7
    v8.position                = glm::vec3( 3.0f, -1.0f, 0.0f );        //vertex 8

    // these are the colors for each vertex
    v1.material_color        = glm::vec3(1.0f, 0.0f, 0.0f);            // red color for bottom left vertex
    v2.material_color        = glm::vec3(0.0f, 1.0f, 0.0f);            // green color for bottom right vertex
    v3.material_color        = glm::vec3(0.0f, 0.0f, 1.0f);            // blue color for top right vertex
    v4.material_color        = glm::vec3(1.0f, 1.0f, 0.0f);            // yellow color for top left vertex
    v5.material_color        = glm::vec3(1.0f, 0.0f, 0.0f);            // red color for bottom left vertex
    v6.material_color        = glm::vec3(0.0f, 1.0f, 0.0f);            // green color for bottom right vertex
    v7.material_color        = glm::vec3(0.0f, 0.0f, 1.0f);            // blue color for top right vertex
    v8.material_color        = glm::vec3(1.0f, 1.0f, 0.0f);            // yellow color for top left vertex

    // add vertex data in counter-clockwise order
    std::vector<VertexStruct> line_data;
    line_data.push_back(v1);
    line_data.push_back(v2);
    line_data.push_back(v3);
    line_data.push_back(v4);
    line_data.push_back(v5);
    line_data.push_back(v6);
    line_data.push_back(v7);
    line_data.push_back(v8);

    // in order to pass the data to the GPU we need to know the total number of bytes our data is
    // in this case position is 3 floats (12 bytes) and color is 3 floats (12 bytes)
    // so each vertex holds 24 bytes.
    // This means that the vertex buffer defines a new vertex in a 24-byte offset. This is called stride.
    // Think of it as a vertex step number
    GLsizei stride = sizeof(VertexStruct);

    // Total size of vertex buffer is: 8 vertices of 24 bytes each
    int total_vertex_byte_size = line_data.size() * stride;

    // we do not use any indices here, so the total number of indices, is the number of vertices in the vertex buffer
    vao_lines_num_vertices = line_data.size();

    // To pass the data to the GPU we do the following:
    // Generate a vertex array object and bind it as active. This will point to the buffer object(s) we will use later on
    // Generate the vertex buffer object(s), set them active and pass the data
    // Since each vertex data might contain more than one attributes, we also need to specify how each vertex data is structured.
    // For example, each vertex here is a struct with one vec3 for the position and one vec3 for the color

    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_lines);

    // Set the VAO active
    glBindVertexArray(vao_lines);

    GLuint vbo_lines = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_lines);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_lines);
    // pass the data to the GPU as a sequence of bytes:
    // - The buffer that the vertex attributes come from is always GL_ARRAY_BUFFER
    // - We also pass the total size and a pointer to the first value in the data buffer
    // - GL_STATIC_DRAW is mostly an indication of how the data will be accessed. It does not affect the rendering result.
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glBufferData.xml
    glBufferData(GL_ARRAY_BUFFER, total_vertex_byte_size, &line_data[0], GL_STATIC_DRAW);

    // to inform OpenGL how each attribute in the vertex data is structured, we use glVertexAttribPointer
    // The following command means that the first vertex attribute (the position) is accessed by OpenGL in the following way:
    // These should be the same with the layout(location = #)​ in type name syntax in the vertex shader
    // - At index 0
    // - is a vec3
    // - has type float
    // - is NOT normalized (-1,1)
    // - the stride value
    // - the offset in the vertex buffer of the vertex attribute
    // to access it at the vertex shader we use layout(location = 0)​ in vec3 <any_name_here>
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // same here
    // to access this at the vertex shader we use layout(location = 1)​ in vec3 <any_name_here>
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    // once we specify the vertex attributes, we need to set them active
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}

// Example of creating quads
// The only difference here is that we create the quads using triangle strips instead of triangles
// The same data is used to render the points and line strips
// Since we do not duplicate any vertices in the vertex buffer, we can also use an index buffer
// to render instead of triangle strips, triangles with an index buffer
void BuildQuadsVAO()
{
    // First, we need to build the data on the CPU side.
    // Then, we pass it onto the GPU

    // to build the vertex data for a quad we need 4 vertices
    VertexStruct v1;
    VertexStruct v2;
    VertexStruct v3;
    VertexStruct v4;
    VertexStruct v5;
    VertexStruct v6;
    VertexStruct v7;
    VertexStruct v8;

    // these are the positions in object space for each vertex
    v1.position                = glm::vec3( -3.0f, 1.0f, 0.0f );        //vertex 1
    v2.position                = glm::vec3( -3.0f, -1.0f, 0.0f );        //vertex 2
    v3.position                = glm::vec3( -1.0f, 1.0f, 0.0f );        //vertex 3
    v4.position                = glm::vec3( -1.0f, -1.0f, 0.0f );        //vertex 4
    v5.position                = glm::vec3( 1.0f, 1.0f, 0.0f );        //vertex 5
    v6.position                = glm::vec3( 1.0f, -1.0f, 0.0f );        //vertex 6
    v7.position                = glm::vec3( 3.0f, 1.0f, 0.0f );        //vertex 7
    v8.position                = glm::vec3( 3.0f, -1.0f, 0.0f );        //vertex 8

    // these are the colors for each vertex
    v1.material_color        = glm::vec3(1.0f, 0.0f, 0.0f);            // red color for bottom left vertex
    v2.material_color        = glm::vec3(0.0f, 1.0f, 0.0f);            // green color for bottom right vertex
    v3.material_color        = glm::vec3(0.0f, 0.0f, 1.0f);            // blue color for top right vertex
    v4.material_color        = glm::vec3(1.0f, 1.0f, 0.0f);            // yellow color for top left vertex
    v5.material_color        = glm::vec3(1.0f, 0.0f, 0.0f);            // red color for bottom left vertex
    v6.material_color        = glm::vec3(0.0f, 1.0f, 0.0f);            // green color for bottom right vertex
    v7.material_color        = glm::vec3(0.0f, 0.0f, 1.0f);            // blue color for top right vertex
    v8.material_color        = glm::vec3(1.0f, 1.0f, 0.0f);            // yellow color for top left vertex

    // add vertex data in counter-clockwise order
    std::vector<VertexStruct> quad_data;
    quad_data.push_back(v1);
    quad_data.push_back(v2);
    quad_data.push_back(v3);
    quad_data.push_back(v4);
    quad_data.push_back(v5);
    quad_data.push_back(v6);
    quad_data.push_back(v7);
    quad_data.push_back(v8);

    // in order to pass the data to the GPU we need to know the total number of bytes our data is
    // in this case position is 3 floats (12 bytes) and color is 3 floats (12 bytes)
    // so each vertex holds 24 bytes.
    // This means that the vertex buffer defines a new vertex in a 24-byte offset. This is called stride.
    // Think of it as a vertex step number
    GLsizei stride = sizeof(VertexStruct);

    // Total size of vertex buffer is: 8 vertices of 24 bytes each
    int total_vertex_byte_size = quad_data.size() * stride;

    // we do not use any indices here, so the total number of indices, is the number of vertices in the vertex buffer
    vao_quads_num_of_indices = quad_data.size();

    // To pass the data to the GPU we do the following:
    // Generate a vertex array object and bind it as active. This will point to the buffer object(s) we will use later on
    // Generate the vertex buffer object(s), set them active and pass the data
    // Since each vertex data might contain more than one attributes, we also need to specify how each vertex data is structured.
    // For example, each vertex here is a struct with one vec3 for the position and one vec3 for the color

    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_quads);
    // Set the VAO active
    glBindVertexArray(vao_quads);

    GLuint vbo_quad = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_quad);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
    // pass the data to the GPU as a sequence of bytes:
    // - The buffer that the vertex attributes come from is always GL_ARRAY_BUFFER
    // - We also pass the total size and a pointer to the first value in the data buffer
    // - GL_STATIC_DRAW is mostly an indication of how the data will be accessed. It does not affect the rendering result.
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glBufferData.xml
    glBufferData(GL_ARRAY_BUFFER, total_vertex_byte_size, &quad_data[0], GL_STATIC_DRAW);

    // to inform OpenGL how each attribute in the vertex data is structured, we use glVertexAttribPointer
    // The following command means that the first vertex attribute (the position) is accessed by OpenGL in the following way:
    // These should be the same with the layout(location = #)​ in type name syntax in the vertex shader
    // - At index 0
    // - is a vec3
    // - has type float
    // - is NOT normalized (-1,1)
    // - the stride value
    // - the offset in the vertex buffer of the vertex attribute
    // to access it at the vertex shader we use layout(location = 0)​ in vec3 <any_name_here>
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // same here
    // to access this at the vertex shader we use layout(location = 1)​ in vec3 <any_name_here>
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    // once we specify the vertex attributes, we need to set them active
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // we can also build an index buffer
    // this will render the vertices according to their index in the vertex buffer
    // the only thing change is the use of a different glDraw command
    GLuint ibo;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &ibo);
    // Bind the newly generated buffer object (note that the glBindBuffer now uses GL_ELEMENT_ARRAY_BUFFER instead of GL_ARRAY_BUFFER)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // create the data
    unsigned int indices_data[18] = {
        0, 1, 2,
        2, 1, 3,
        2, 3, 4,
        4, 3, 5,
        4, 5, 6,
        6, 5, 7};

    vao_quads_num_of_indices2 = 18;

    // pass the data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao_quads_num_of_indices2 * sizeof(unsigned int), &indices_data[0], GL_STATIC_DRAW);
    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}

void BuildTriangleFanVAO()
{
    std::vector<VertexStruct> triangle_fan_data;

    VertexStruct v_center;
    v_center.position = glm::vec3(0.0f, 0.0f, 0.0f);
    v_center.material_color = glm::vec3(1.0f, 1.0f, 1.0f);
    triangle_fan_data.push_back(v_center);

    // number of triangles
    float tesselation = 16.0f;
    float radius = 2.0f;
    // each triangle will be 360/tesselation degrees wide (e.g. tesselation 16 = 22.5 degrees each)
    float angle_step = glm::pi<float>() * 2.0f / tesselation;
    for(int step = 0; step <= tesselation; step++)
    {
        float rad_angle = step * angle_step;
        // cosine and sine use radians
        float x = radius * glm::cos(rad_angle);
        float y = radius * glm::sin(rad_angle);
        VertexStruct v;
        v.position = glm::vec3(x, y, 0.0f);
        v.material_color = glm::vec3(x/radius, y/radius, 0.0f);
        triangle_fan_data.push_back(v);
    }

    // in order to pass the data to the GPU we need to know the total number of bytes our data is
    // in this case position is 3 floats (12 bytes) and color is 3 floats (12 bytes)
    // so each vertex holds 24 bytes.
    // This means that the vertex buffer defines a new vertex in a 24-byte offset. This is called stride.
    // Think of it as a vertex step number
    GLsizei stride = sizeof(VertexStruct);

    // Total size of vertex buffer is: num of vertices of 24 bytes each
    int total_vertex_byte_size = triangle_fan_data.size() * stride;

    // we do not use any indices here, so the total number of indices, is the number of vertices in the vertex buffer
    vao_triangle_fan_num_of_indices = triangle_fan_data.size();

    // To pass the data to the GPU we do the following:
    // Generate a vertex array object and bind it as active. This will point to the buffer object(s) we will use later on
    // Generate the vertex buffer object(s), set them active and pass the data
    // Since each vertex data might contain more than one attributes, we also need to specify how each vertex data is structured.
    // For example, each vertex here is a struct with one vec3 for the position and one vec3 for the color

    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_triangle_fan);
    // Set the VAO active
    glBindVertexArray(vao_triangle_fan);

    GLuint vbo_triangle_fan = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_triangle_fan);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_fan);
    // pass the data to the GPU as a sequence of bytes:
    // - The buffer that the vertex attributes come from is always GL_ARRAY_BUFFER
    // - We also pass the total size and a pointer to the first value in the data buffer
    // - GL_STATIC_DRAW is mostly an indication of how the data will be accessed. It does not affect the rendering result.
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glBufferData.xml
    glBufferData(GL_ARRAY_BUFFER, total_vertex_byte_size, &triangle_fan_data[0], GL_STATIC_DRAW);

    // to inform OpenGL how each attribute in the vertex data is structured, we use glVertexAttribPointer
    // The following command means that the first vertex attribute (the position) is accessed by OpenGL in the following way:
    // These should be the same with the layout(location = #)​ in type name syntax in the vertex shader
    // - At index 0
    // - is a vec3
    // - has type float
    // - is NOT normalized (-1,1)
    // - the stride value
    // - the offset in the vertex buffer of the vertex attribute
    // to access it at the vertex shader we use layout(location = 0)​ in vec3 <any_name_here>
    // For more information, look here http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribPointer.xml
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    // same here
    // to access this at the vertex shader we use layout(location = 1)​ in vec3 <any_name_here>
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    // once we specify the vertex attributes, we need to set them active
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}

void BuildSphereVAO(float sphere_radius, int longitude_steps, int latitude_steps)
{
    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_sphere);
    // Set the VAO active
    glBindVertexArray(vao_sphere);

    // add vertex data
    std::vector<glm::vec3> sphere_data;

    if (longitude_steps < 2) longitude_steps = 2;
    if (latitude_steps < 4) latitude_steps = 4;

    float phi_step = 2 * glm::pi<float>() / float(latitude_steps);
    float theta_step = glm::pi<float>() / float(longitude_steps);
    float phi = 0;
    float theta = 0;

    glm::vec3 color = glm::vec3(0.5f, 0.5f, 0.5f);

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

            glm::vec3 top_left_vertex;
            top_left_vertex.x = sphere_radius * sintheta * cosphi;
            top_left_vertex.z = sphere_radius * sintheta * sinphi;
            top_left_vertex.y = sphere_radius * costheta;

            glm::vec3 top_right_vertex;
            top_right_vertex.x = sphere_radius * sintheta * cosphi_plus_step;
            top_right_vertex.z = sphere_radius * sintheta * sinphi_plus_step;
            top_right_vertex.y = sphere_radius * costheta;

            glm::vec3 bottom_left_vertex;
            bottom_left_vertex.x = sphere_radius * sintheta_plus_step * cosphi;
            bottom_left_vertex.z = sphere_radius * sintheta_plus_step * sinphi;
            bottom_left_vertex.y = sphere_radius * costheta_plus_step;

            glm::vec3 bottom_right_vertex;
            bottom_right_vertex.x = sphere_radius * sintheta_plus_step * cosphi_plus_step;
            bottom_right_vertex.z = sphere_radius * sintheta_plus_step * sinphi_plus_step;
            bottom_right_vertex.y = sphere_radius * costheta_plus_step;

            sphere_data.push_back(bottom_left_vertex);
            sphere_data.push_back(color);
            sphere_data.push_back(bottom_right_vertex);
            sphere_data.push_back(color);
            sphere_data.push_back(top_right_vertex);
            sphere_data.push_back(color);
            sphere_data.push_back(top_right_vertex);
            sphere_data.push_back(color);
            sphere_data.push_back(top_left_vertex);
            sphere_data.push_back(color);
            sphere_data.push_back(bottom_left_vertex);
            sphere_data.push_back(color);

            //PrintToOutputWindow("Phi: %.2f -> %.2f, Theta: %.2f -> %.2f", glm::degrees(phi), glm::degrees(phi + phi_step), glm::degrees(theta), glm::degrees(theta + theta_step));
            phi += phi_step;
        }
        theta += theta_step;
    }

    GLsizei stride = (3 + 3) * sizeof(GLfloat);
    GLsizeiptr datasize = sphere_data.size() * sizeof(glm::vec3);
    vao_sphere_indices = (int)sphere_data.size() / 2;

    GLuint vbo_sphere = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_sphere);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_sphere);
    // pass the data to the GPU as a sequence of bytes
    glBufferData(GL_ARRAY_BUFFER, datasize, &sphere_data[0], GL_STATIC_DRAW);
    // inform OpenGL how this data is structured
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}