//----------------------------------------------------//
//                                                    //
// File: ShaderGLSL.cpp                               //
// Handles basic GLSL functionality                   //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

// includes ////////////////////////////////////////
#include "HelpLib.h"    // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "ShaderGLSL.h" // - Header file for our shader class

// defines /////////////////////////////////////////


// Constructor
ShaderGLSL::ShaderGLSL(const char* name, bool has_geometry_shader) :
    m_name(name),
    m_has_geom_shader(has_geometry_shader),
    m_loaded(false),
    m_program(unsigned int(-1))
{

}

// Destructor
ShaderGLSL::~ShaderGLSL()
{

}

// other functions
// Load the shader file and compile
bool ShaderGLSL::LoadAndCompile()
{
    std::string error = "";
    if (!LoadShader(error))
        PrintToOutputWindow("Shader: %s could not be created.", m_name.c_str());
    else
    {
        PrintToOutputWindow("Shader: %s created.", m_name.c_str());
        m_loaded = true;
    }
    return m_loaded;
}

// This function destroys any internal OpenGL allocations
// Shader program needs to be detached from the shader types and deleted
void ShaderGLSL::Destroy()
{
    m_has_geom_shader = false;
    // detach the shaders
    glDetachShader(m_program, m_vertex_shader);
    glError();
    if (m_has_geom_shader)
        glDetachShader(m_program, m_geometry_shader);
    glError();
    glDetachShader(m_program, m_fragment_shader);
    glError();
    UnloadShader();
}

// Delete the detached program and shader types
void ShaderGLSL::UnloadShader(void)
{
    if (m_program == unsigned int(-1)) return;

    m_loaded = false;

    glDeleteShader(m_vertex_shader);
    glError();
    if (m_has_geom_shader)
        glDeleteShader(m_geometry_shader);
    glError();
    glDeleteShader(m_fragment_shader);
    glError();
    glDeleteProgram(m_program);
    glError();
    m_program = unsigned int(-1);
    m_vertex_shader = unsigned int(-1);
    m_geometry_shader = unsigned int(-1);
    m_fragment_shader = unsigned int(-1);
}

// This function allows reloading a shader at runtime (useful for debugging purposes)
void ShaderGLSL::ReloadShader(void)
{
    // detach the shaders
    glDetachShader(m_program, m_vertex_shader);

    if (m_has_geom_shader)
        glDetachShader(m_program, m_geometry_shader);

    glDetachShader(m_program, m_fragment_shader);

    LoadAndCompile();

    glError();
}

// Shader loading requires a sequence of operations
// - Create an OpenGL object to get an ID for the shader using glCreateProgram. This object will contain all shader types (vertex, geometry, fragment) for this shader object
// - Create an OpenGL shader to get an ID for each shader type (vertex, geometry, fragment)
// - Load the shader files to memory buffers
// - Associate each buffer to a shader type
// - Pass the buffers to the GLSL compiler, compile and link them (similar to how C++ compiles and links files)
// - If compilation and linking is successful, attach the shader types to the shader object, otherwise print the appropriate error
bool ShaderGLSL::LoadShader(std::string& error)
{
    glError();
    UnloadShader();

    // Create the OpenGL shader object and shader types
    if (m_program == unsigned int(-1))
    {
        m_program = glCreateProgram();

        m_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        if (m_has_geom_shader)
            m_geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
        m_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    }

    // Load the shader files to memory buffers
    std::string* vertex_source_str = new std::string("");
    std::string* geometry_source_str = new std::string("");
    std::string* fragment_source_str = new std::string("");

    std::string full_shader_path = "..\\Shaders\\";

    bool read_result = ReadShader(full_shader_path, m_name, error, vertex_source_str, geometry_source_str, fragment_source_str);
    if (!read_result) return false;

    const char *vertex_source = vertex_source_str->c_str();
    const char *geometry_source = NULL;
    if (strcmp(geometry_source_str->c_str(), "") != 0) geometry_source = geometry_source_str->c_str();
    const char *fragment_source = fragment_source_str->c_str();

    // Associate each buffer to a shader type
    // Pass the buffers to the GLSL compiler, compile and link them (similar to how C++ compiles and links files)
    // If compilation and linking is successful, attach the shader types to the shader object, otherwise print the appropriate error
    glShaderSource(m_vertex_shader, 1, &vertex_source, NULL);
    if (geometry_source)
        glShaderSource(m_geometry_shader, 1, &geometry_source, NULL);
    glShaderSource(m_fragment_shader, 1, &fragment_source, NULL);

    GLint compiled = false;
    GLint linked = false;
    glCompileShader(m_vertex_shader);
    glGetShaderiv(m_vertex_shader, GL_COMPILE_STATUS, &compiled);
    ShaderInfoLog(m_vertex_shader, GL_VERTEX_SHADER);
    if (geometry_source)
    {
        glCompileShader(m_geometry_shader);
        glGetShaderiv(m_geometry_shader, GL_COMPILE_STATUS, &compiled);
        ShaderInfoLog(m_geometry_shader, GL_GEOMETRY_SHADER);
    }
    glCompileShader(m_fragment_shader);
    glGetShaderiv(m_fragment_shader, GL_COMPILE_STATUS, &compiled);
    ShaderInfoLog(m_fragment_shader, GL_FRAGMENT_SHADER);

    if (compiled)
    {
        glAttachShader(m_program, m_vertex_shader);
        if (geometry_source)
            glAttachShader(m_program, m_geometry_shader);
        glAttachShader(m_program, m_fragment_shader);

        glLinkProgram(m_program);
        glGetProgramiv(m_program, GL_LINK_STATUS, &linked);

        ProgramInfoLog(linked);
    }

    // release the dynamically allocated buffer data
    SAFE_DELETE(vertex_source_str)
    SAFE_DELETE(geometry_source_str)
    SAFE_DELETE(fragment_source_str)

    // check for OpenGL errors
    glError();

    return (compiled && linked);
}

// If the shader program is not linked successfully, print the generated error
void ShaderGLSL::ProgramInfoLog(GLint linked)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1 && !linked)
    {
        infoLog = new char[infologLength];
        glGetProgramInfoLog(m_program, infologLength, &charsWritten, infoLog);
        PrintToOutputWindow("Shader %s program error: %s.", m_name.c_str(), infoLog);
        delete infoLog;
    }
}

// If the shader type is not linked successfully, print the generated error
void ShaderGLSL::ShaderInfoLog(GLuint shader, GLint shader_type)
{
    GLint infologLength = 0;
    GLint charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 1)
    {
        infoLog = new char[infologLength];
        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
        if (shader_type == GL_VERTEX_SHADER)
        {
            PrintToOutputWindow("\nVertexShader %s.vert compilation error:%s", m_name.c_str(), infoLog);
        }
        else if (shader_type == GL_GEOMETRY_SHADER)
        {
            PrintToOutputWindow("\nGeometry %s.geom compilation error:%s", m_name.c_str(), infoLog);
        }
        else if (shader_type == GL_FRAGMENT_SHADER)
        {
            PrintToOutputWindow("\nPixelShader %s.frag compilation error:%s", m_name.c_str(), infoLog);
        }
        delete infoLog;
    }
}

// Load the shader file to memory buffers
bool ShaderGLSL::ReadShader(std::string& shader_path, std::string& name, std::string& error, std::string* vertex_source, std::string* geometry_source, std::string* fragment_source)
{
    std::string fileName = "";
    char* v_source = nullptr;
    fileName.clear();
    fileName = shader_path + name + ".vert";
    bool vert_exists = readFile(fileName, &v_source);

    if (vert_exists)
        *vertex_source = v_source;

    SAFE_DELETE(v_source);

    if (!vert_exists)
    {
        PrintToOutputWindow("Shader: %s.vert could not be found in path.", m_name.c_str(), shader_path.c_str());
        return false;
    }

    if (m_has_geom_shader)
    {
        char* g_source = nullptr;
        fileName.clear();
        fileName = shader_path + name + ".geom";
        bool geom_exists = readFile(fileName, &g_source);
        if (geom_exists)
            *geometry_source = g_source;
        SAFE_DELETE(g_source);
        if (!geom_exists)
        {
            PrintToOutputWindow("Shader: %s.geom could not be found in path.", m_name.c_str(), shader_path.c_str());
            return false;
        }
    }

    char* f_source = nullptr;
    fileName.clear();
    fileName = shader_path + name + ".frag";
    bool frag_exists = readFile(fileName, &f_source);
    if (frag_exists)
        *fragment_source = f_source;
    SAFE_DELETE(f_source);
    if (!frag_exists)
    {
        PrintToOutputWindow("Shader: %s.frag could not be found in path.", m_name.c_str(), shader_path.c_str());
        return false;
    }

    return true;
}

// eof ///////////////////////////////// class ShaderGLSL