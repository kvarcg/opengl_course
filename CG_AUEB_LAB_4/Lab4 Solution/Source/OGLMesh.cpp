//----------------------------------------------------//
//                                                    //
// File: OGLMesh.cpp                                  //
// OGLMesh holds the OpenGL 3.3 compatible vertex     //
// data structure and texture information             //
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
#include "OGLMesh.h"    // - Header file for the OGLMesh class
#include "ShaderGLSL.h" // - Header file for the ShaderGLSL class

// Constructor
OGLMesh::OGLMesh(std::string& filename, std::string& path):
    m_fileName(filename),
    m_path(path),
    num_total_vertices(0),
    num_total_primitives(0),
    num_total_elements(0),
    num_elements(0),
    num_vertexdata(0),
    indexdata(nullptr),
    vertexdata(nullptr),
    elements(nullptr),
    is_dynamic(false),
    updated(false)
{

}

// Destructor
OGLMesh::~OGLMesh(void)
{
    init();
}

// other functions
void OGLMesh::init()
{
    if (!released)
    {
        release();
    }

    num_total_vertices = 0;
    num_total_primitives = 0;
    num_total_elements = 0;

    num_elements = 0;
    num_vertexdata = 0;
    SAFE_DELETE_ARRAY_POINTER(indexdata);
    SAFE_DELETE_ARRAY_POINTER(vertexdata);
    SAFE_DELETE_ARRAY_POINTER(elements);

    for (unsigned int i = 0; i < materials.size(); ++i)
    {
        SAFE_DELETE(materials[i]);
    }

    materials.clear();

    is_dynamic = false;
    updated = false;

    // shader loading code
    //m_shader_glsl = new ShaderGLSL("BasicGeometry");
    m_shader_glsl = new ShaderGLSL("BasicLighting");
    bool shader_loaded = m_shader_glsl->LoadAndCompile();
    if (!shader_loaded) return;
    m_program_id = m_shader_glsl->GetProgram();
    m_uniform_time = glGetUniformLocation(m_program_id, "uniform_time");
    m_uniform_m = glGetUniformLocation(m_program_id, "uniform_m");
    m_uniform_v = glGetUniformLocation(m_program_id, "uniform_v");
    m_uniform_p = glGetUniformLocation(m_program_id, "uniform_p");
    m_uniform_color = glGetUniformLocation(m_program_id, "uniform_material_color");
}

bool OGLMesh::loadToOpenGL(OBJMesh &_mesh)
{
    updated = false;

    init();

    if (!m_shader_glsl->Loaded())
        return false;

    size_t s = _mesh.materials.size();
    materials.reserve(_mesh.materials.size());
    for (unsigned int i = 0; i < _mesh.materials.size(); ++i)
    {
        materials.push_back(_mesh.materials[i]);
    }

    for (unsigned int i=0; i<_mesh.num_elements; i++)
    {
        PrimitiveGroup& group = _mesh.elements[i];
        num_elements++;
        num_vertexdata += group.num_primitives * 3;
    }

    // allocate buffers
    if (num_vertexdata > 0)
    {
        indexdata = new GLuint[num_vertexdata];
        vertexdata = new VertexData[num_vertexdata];
        elements = new ElementGroup[num_elements];
    }

    unsigned int voffset = 0;
    unsigned int eoffset = 0;

    for (unsigned int i=0; i<_mesh.num_elements; i++)
    {
        PrimitiveGroup& group = _mesh.elements[i];

        elements[eoffset].material_index = group.material_index;
        elements[eoffset].triangles = group.num_primitives;
        elements[eoffset].start_index = voffset;

        long curtangent = 0;
        for (unsigned int j=0; j<(unsigned int)group.num_primitives; j++)
        {
            Triangle& tr = group.primitives[j];
            for (int k=0; k<3; k++)
            {
                vertexdata[voffset].position[0] = tr.vertex[k][0];
                vertexdata[voffset].position[1] = tr.vertex[k][1];
                vertexdata[voffset].position[2] = tr.vertex[k][2];
                vertexdata[voffset].normal[0] = tr.normal[k][0];
                vertexdata[voffset].normal[1] = tr.normal[k][1];
                vertexdata[voffset].normal[2] = tr.normal[k][2];
                vertexdata[voffset].tangent[0] = tr.tangent[k][0];
                vertexdata[voffset].tangent[1] = tr.tangent[k][1];
                vertexdata[voffset].tangent[2] = tr.tangent[k][2];
                vertexdata[voffset].texcoord0[0] = tr.texcoord[0][k][0];
                vertexdata[voffset].texcoord0[1] = tr.texcoord[0][k][1];
                vertexdata[voffset].texcoord1[0] = tr.texcoord[1][k][0];
                vertexdata[voffset].texcoord1[1] = tr.texcoord[1][k][1];
                indexdata[voffset] = voffset;
                voffset++;
            }
            num_total_vertices+=3;
            num_total_primitives++;
        }
        elements[eoffset].end_index = glm::max(voffset-1, unsigned int(0));
        eoffset++;
        num_total_elements++;
    }

    if (num_vertexdata==0)
    {
        PrintToOutputWindow("No data in mesh to load to OpenGL. Should not get here. Exiting");
        return false;
    }

    glGenVertexArrays(1, &(vao));
    glBindVertexArray(vao);

    glGenBuffers(1, &(vbo));
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (!is_dynamic)
        glBufferData(GL_ARRAY_BUFFER, num_vertexdata*sizeof(VertexData), vertexdata, GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, num_vertexdata*sizeof(VertexData), vertexdata, GL_STREAM_DRAW);

    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),  (GLvoid*)(0));
    glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),  (GLvoid*)(3*sizeof(GLfloat)));
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),  (GLvoid*)(6*sizeof(GLfloat)));
    glVertexAttribPointer((GLuint)3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData),  (GLvoid*)(8*sizeof(GLfloat)));
    glVertexAttribPointer((GLuint)4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData),  (GLvoid*)(10*sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glGenBuffers(1, &(ibo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    if (!is_dynamic)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_vertexdata*sizeof(GLuint), indexdata, GL_STATIC_DRAW);
    else
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_vertexdata*sizeof(GLuint), indexdata, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);

    bool hasGLError = glError();

    if (hasGLError)
    {
        PrintToOutputWindow("An OpenGL error was generated while loading vertex data to the GPU. Skipping");
        return false;
    }

    released = false;

    updated = true;
    return true;
}

void OGLMesh::display()
{

}

void OGLMesh::release()
{
    if (released) return;

    glError();

    glBindVertexArray(0);
    for (int i=0;i<7;i++)
        glDisableVertexAttribArray(i);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glError();
    released = true;
}

void OGLMesh::dump(void)
{
    PrintToOutputWindow("Model info: %s", m_fileName.c_str());
    PrintToOutputWindow("\tNum of Elements : %lu\n", num_elements);
    PrintToOutputWindow("\tNum of Vertices : %lu", num_total_vertices);

    PrintToOutputWindow("\n\tNum of Materials    : %lu\n", materials.size());
    for (unsigned int i = 0; i < materials.size(); i++)
        materials[i]->dump ();
}

// eof ///////////////////////////////// class OGLMesh