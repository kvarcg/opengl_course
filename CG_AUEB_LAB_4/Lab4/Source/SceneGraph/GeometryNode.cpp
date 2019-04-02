//----------------------------------------------------//
//                                                    //
// File: GeometryNode.cpp                             //
// This scene graph is a basic example for the        //
// object relational management of the scene          //
// This holds the geometry node which is used for     //
// rendering geometric data                           //
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
#include "../HelpLib.h"     // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "GeometryNode.h"   // - Header file for the GeometryNode class
#include "Root.h"           // - Header file for the Root class
#include "../OGLMesh.h"     // - Header file for the OGLMesh class
#include "../OBJMaterial.h" // - Header file for the OBJMaterial class

// defines /////////////////////////////////////////

GeometryNode::GeometryNode(const char* name, OGLMesh* ogl_mesh):
Node(name)
{
    m_ogl_mesh = ogl_mesh;
}

GeometryNode::~GeometryNode()
{

}

void GeometryNode::Update()
{
    Node::Update();
}

void GeometryNode::Draw()
{
    if (m_ogl_mesh == nullptr)
        return;

    // bind the VAO
    glBindVertexArray(m_ogl_mesh->vao);

    // get the world transformation (hierarchically)
    glm::mat4x4& M = GetTransform();
    // get the view transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& V = m_root->GetViewMat();
    // get the projection transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& P = m_root->GetProjectionMat();

    std::vector<OBJMaterial*>& materials = m_ogl_mesh->materials;

    // set the shader active
    glUseProgram(m_ogl_mesh->m_program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(m_ogl_mesh->m_uniform_m, 1, false, &M[0][0]);
    glUniformMatrix4fv(m_ogl_mesh->m_uniform_v, 1, false, &V[0][0]);
    glUniformMatrix4fv(m_ogl_mesh->m_uniform_p, 1, false, &P[0][0]);

    // loop through all the elements
    for (GLint i=0; i < m_ogl_mesh->num_elements; i++)
    {
        if (m_ogl_mesh->elements[i].triangles==0)
            continue;

        // Material and texture goes here.
        int mtrIdx = m_ogl_mesh->elements[i].material_index;
        OBJMaterial& cur_material = *materials[mtrIdx];

        // use the material color
        glUniform4f(m_ogl_mesh->m_uniform_color, cur_material.m_diffuse[0], cur_material.m_diffuse[1], cur_material.m_diffuse[2], 1.0f);

        // draw within a range in the index buffer
        glDrawRangeElements(
        GL_TRIANGLES,
        m_ogl_mesh->elements[i].start_index,
        m_ogl_mesh->elements[i].start_index+m_ogl_mesh->elements[i].triangles*3,
        m_ogl_mesh->elements[i].triangles*3,
        GL_UNSIGNED_INT,
        (void*)(m_ogl_mesh->elements[i].start_index*sizeof(GLuint))
        );
    }

    glBindVertexArray(0);
}

void GeometryNode::Init()
{
    Node::Init();
}

// eof ///////////////////////////////// class GeometryNode