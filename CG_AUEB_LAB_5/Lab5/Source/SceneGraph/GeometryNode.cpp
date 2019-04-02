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
#include "../ShaderGLSL.h"  // - Header file for GLSL objects

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

void GeometryNode::Draw(int shader_type)
{
    if (m_ogl_mesh == nullptr)
        return;

    // SHADER TYPE 0 - use directional light shader
    // SHADER TYPE 1 - use omnidirectional light shader
    // The only difference is in the uniforms we need to pass to each shader

    if (shader_type == 0)
    {
        DrawUsingDirectionalLight();
    }
    else if (shader_type == 1)
    {
        DrawUsingOmniLight();
    }
}

void GeometryNode::Init()
{
    Node::Init();
}

void GeometryNode::DrawUsingDirectionalLight()
{
    // get the world transformation (hierarchically)
    glm::mat4x4& M = GetTransform();
    // get the view transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& V = m_root->GetViewMat();
    // get the projection transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& P = m_root->GetProjectionMat();

    // get the directional light parameters from the root node
    DirectionalLight* light = m_root->GetActiveDirectionalLight();
    // exit if we have not set it
    if (light == nullptr) return;

    // get the shader from the root node
    DirectionalLightShader* shader = m_root->GetDirectionalLightShader();

    // set the shader active
    glUseProgram(shader->program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(shader->uniform_m, 1, false, &M[0][0]);
    glUniformMatrix4fv(shader->uniform_v, 1, false, &V[0][0]);
    glUniformMatrix4fv(shader->uniform_p, 1, false, &P[0][0]);

    // get the normal matrix which transforms normals from OCS to ECS and pass it to the shader
    glm::mat4x4 normal_matrix = glm::inverse(glm::transpose(V * M));
    glUniformMatrix4fv(shader->uniform_normal_matrix_ecs, 1, false, &normal_matrix[0][0]);

    // for directional lights we only need to pass the light direction and the light color
    // the light direction is passed in ECS coordinates
    // since this is a vector and not a point we transform it by the world view matrix by keeping the w coordinate to 0
    // in order for the vector to not be affected by translations
    glm::vec4 light_direction_wcs = light->m_transformed_direction;
    glm::vec4 light_direction_ecs = V * light_direction_wcs;
    // we need to renormalize the light direction since the world matrix might contain scaling transformations
    light_direction_ecs = glm::normalize(light_direction_ecs);
    // pass it to the shader as a uniform vec3
    glUniform3f(shader->uniform_light_direction_ecs, light_direction_ecs.x, light_direction_ecs.y, light_direction_ecs.z);
    // the light color is passed as a uniform vec3
    glUniform3f(shader->uniform_light_color, light->m_color.x, light->m_color.y, light->m_color.z);

    // bind the VAO
    glBindVertexArray(m_ogl_mesh->vao);

    // loop through all the elements
    for (GLint i=0; i < m_ogl_mesh->num_elements; i++)
    {
        if (m_ogl_mesh->elements[i].triangles==0)
            continue;

        // Material and texture goes here.
        int mtrIdx = m_ogl_mesh->elements[i].material_index;
        OBJMaterial& cur_material = *m_ogl_mesh->materials[mtrIdx];

        // use the material color
        glUniform4f(shader->uniform_color, cur_material.m_diffuse[0], cur_material.m_diffuse[1], cur_material.m_diffuse[2], cur_material.m_opacity);

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
    glUseProgram(0);
}

void GeometryNode::DrawUsingOmniLight()
{
    // get the world transformation (hierarchically)
    glm::mat4x4& M = GetTransform();
    // get the view transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& V = m_root->GetViewMat();
    // get the projection transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& P = m_root->GetProjectionMat();

    // get the omni light parameters from the root node
    OmniLight* light = m_root->GetActiveOmnilLight();
    // exit if we have not set it
    if (light == nullptr) return;

    // get the shader from the root node
    OmniLightShader* shader = m_root->GetOmniLightShader();

    // set the shader active
    glUseProgram(shader->program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(shader->uniform_m, 1, false, &M[0][0]);
    glUniformMatrix4fv(shader->uniform_v, 1, false, &V[0][0]);
    glUniformMatrix4fv(shader->uniform_p, 1, false, &P[0][0]);

    // get the normal matrix which transforms normals from OCS to ECS and pass it to the shader
    glm::mat4x4 normal_matrix = glm::inverse(glm::transpose(V * M));
    glUniformMatrix4fv(shader->uniform_normal_matrix_ecs, 1, false, &normal_matrix[0][0]);

    // for omni lights we only need to pass the light position and the light color
    // the light direction is passed in ECS coordinates so we need to transform it first
    glm::vec4 light_position_wcs = light->m_tranformed_position;
    glm::vec4 light_position_ecs = V * light_position_wcs;
    // the position is passed as a uniform vec3
    glUniform3f(shader->uniform_light_position_ecs, light_position_ecs.x, light_position_ecs.y, light_position_ecs.z);
    // the light color is passed as a uniform vec3
    glUniform3f(shader->uniform_light_color, light->m_color.x, light->m_color.y, light->m_color.z);

    // bind the VAO
    glBindVertexArray(m_ogl_mesh->vao);

    // loop through all the elements
    for (GLint i=0; i < m_ogl_mesh->num_elements; i++)
    {
        if (m_ogl_mesh->elements[i].triangles==0)
            continue;

        // Material and texture goes here.
        int mtrIdx = m_ogl_mesh->elements[i].material_index;
        OBJMaterial& cur_material = *m_ogl_mesh->materials[mtrIdx];

        // use the material color
        glUniform4f(shader->uniform_color, cur_material.m_diffuse[0], cur_material.m_diffuse[1], cur_material.m_diffuse[2], cur_material.m_opacity);

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
    glUseProgram(0);
}
// eof ///////////////////////////////// class GeometryNode