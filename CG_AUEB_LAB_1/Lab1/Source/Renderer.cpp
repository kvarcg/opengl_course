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

#include "ShaderGLSL.h" // - Header file for GLSL objects
#include "Renderer.h"   // - Header file for our OpenGL functions

// vertex array objects
GLuint vao_triangle;
GLuint vao_quad;

// shader objects
ShaderGLSL* bgs_glsl = nullptr;
GLint bgs_program_id;

void BuildTriangleVAO();
void BuildQuadVAO();

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
    vao_quad = 0;

    // create a basic GLSL shader for rendering simple primitives with
    // different colors per vertex
    bgs_glsl = new ShaderGLSL("BasicGeometry");
    bool shader_loaded = bgs_glsl->LoadAndCompile();
    // if the shader has not been loaded correctly, the VS Output window will contain the errors produced
    // and the application will exit
    if (!shader_loaded) return false;
    // once compiled, OpenGL creates a program ID, used to identify the shader. Store this for later use during rendering.
    bgs_program_id = bgs_glsl->GetProgram();

    // build the data structures for a triangle and a quad
    BuildTriangleVAO();
    BuildQuadVAO();

    // check if we have generated any OpenGL errors
    glError();

    // return true if everything is OK!
    return true;
}

// Render function. Every time our window has to be drawn, this is called.
void Render(void)
{
    // First this, we clear our depth and color buffers.
    // We can clear both using an | operand to create the buffer-clear mask.

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set the shader active
    glUseProgram(bgs_program_id);

    // Draw a triangle
    /*
    glBindVertexArray(vao_triangle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    */

    // Draw a quad
    /*
    glBindVertexArray(vao_quad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    */

    // tell OpenGL that no shader object is active
    glUseProgram(0);
    // tell OpenGL that no vertex array is active
    glBindVertexArray(0);

    // Remember that we are using double-buffering, all the drawing we just did
    // took place in the "hidden" back-buffer. Calling glutSwapBuffers makes the
    // back buffer "visible".
    glutSwapBuffers();

    glError();
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


// Resize callback function
// The parameters indicate the new width and height.
void Resize(int width, int height)
{
    // Hack to void zero height
    if (height == 0) height = 1;

    // Set up the viewport
    glViewport(0, 0, width, height);
}

void BuildTriangleVAO()
{
    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_triangle);
    // Set the VAO active
    glBindVertexArray(vao_triangle);

    // counter-clockwise order
    glm::vec4 bottom_left_vertex  = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f); // bottom left vertex
    glm::vec4 bottom_right_vertex = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);  // bottom right vertex
    glm::vec4 top_vertex          = glm::vec4(0.0f,  1.0f, 0.0f, 1.0f);  // top vertex

    // Same color for all vertices
    glm::vec4 bottom_left_color   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom left vertex
    glm::vec4 bottom_right_color  = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom right vertex
    glm::vec4 top_color           = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for top vertex

    /*
    // Per-vertex coloring
    glm::vec4 bottom_left_color   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom left vertex
    glm::vec4 bottom_right_color  = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);   // green color for bottom right vertex
    glm::vec4 top_color           = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);   // blue color for top vertex
    */

    // add vertex data
    std::vector<glm::vec4> triangle_data;
    triangle_data.push_back(bottom_left_vertex);
    triangle_data.push_back(bottom_left_color);
    triangle_data.push_back(bottom_right_vertex);
    triangle_data.push_back(bottom_right_color);
    triangle_data.push_back(top_vertex);
    triangle_data.push_back(top_color);

    GLsizei stride = (4 + 4) * sizeof(GLfloat);
    GLsizeiptr datasize = triangle_data.size() * sizeof(glm::vec4);

    GLuint vbo_triangle = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_triangle);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    // pass the data to the GPU as a sequence of bytes
    glBufferData(GL_ARRAY_BUFFER, datasize, &triangle_data[0], GL_STATIC_DRAW);
    // inform OpenGL how this data is structured
    glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}

void BuildQuadVAO()
{
    // Generate a vertex array object (VAO) to point to buffer objects
    glGenVertexArrays(1, &vao_quad);
    // Set the VAO active
    glBindVertexArray(vao_quad);

    // counter-clockwise order
    glm::vec4 bottom_left_vertex  = glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f); // bottom left vertex
    glm::vec4 bottom_right_vertex = glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);  // bottom right vertex
    glm::vec4 top_right_vertex      = glm::vec4(1.0f,  1.0f, 0.0f, 1.0f);  // top right vertex
    glm::vec4 top_left_vertex      = glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f); // top left vertex

    // Same color for all vertices
    glm::vec4 bottom_left_color   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom left vertex
    glm::vec4 bottom_right_color  = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom right vertex
    glm::vec4 top_right_color       = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for top right vertex
    glm::vec4 top_left_color       = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for top left vertex

    /*
    // Per-vertex coloring
    glm::vec4 bottom_left_color   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);   // red color for bottom left vertex
    glm::vec4 bottom_right_color  = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);   // green color for bottom right vertex
    glm::vec4 top_left_color       = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);   // blue color for top right vertex
    glm::vec4 top_right_color       = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);   // yellow color for top left vertex
    */

    // add vertex data
    std::vector<glm::vec4> quad_data;
    quad_data.push_back(bottom_left_vertex);
    quad_data.push_back(bottom_left_color);
    quad_data.push_back(bottom_right_vertex);
    quad_data.push_back(bottom_right_color);
    quad_data.push_back(top_right_vertex);
    quad_data.push_back(top_right_color);
    quad_data.push_back(top_right_vertex);
    quad_data.push_back(top_right_color);
    quad_data.push_back(top_left_vertex);
    quad_data.push_back(top_left_color);
    quad_data.push_back(bottom_left_vertex);
    quad_data.push_back(bottom_left_color);

    GLsizei stride = (4 + 4) * sizeof(GLfloat);
    GLsizeiptr datasize = quad_data.size() * sizeof(glm::vec4);

    GLuint vbo_quad = 0;
    // Generate a buffer object which holds the vertex data
    glGenBuffers(1, &vbo_quad);
    // Bind the newly generated buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo_quad);
    // pass the data to the GPU as a sequence of bytes
    glBufferData(GL_ARRAY_BUFFER, datasize, &quad_data[0], GL_STATIC_DRAW);
    // inform OpenGL how this data is structured
    glVertexAttribPointer((GLuint)0, 4, GL_FLOAT, GL_FALSE, stride, 0);
    glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // set the VAO inactive
    glBindVertexArray(0);

    // check for OpenGL errors
    glError();
}