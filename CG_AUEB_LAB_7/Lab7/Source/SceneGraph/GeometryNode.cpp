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
#include "../HelpLib.h"         // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "GeometryNode.h"       // - Header file for the GeometryNode class
#include "Root.h"               // - Header file for the Root class
#include "../OBJ/OGLMesh.h"     // - Header file for the OGLMesh class
#include "../OBJ/OBJMaterial.h" // - Header file for the OBJMaterial class
#include "../OBJ/Texture.h"     // - Header file for the Texture class
#include "../ShaderGLSL.h"      // - Header file for GLSL objects

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

    // SHADER TYPE 0 - use spotlight shader
    // SHADER TYPE 1 - render to shadow map shader
    if (shader_type == 0)
    {
        DrawUsingSpotLight();
    }
    else if (shader_type == 1)
    {
        DrawToShadowMap();
    }
}

void GeometryNode::Init()
{
    Node::Init();
}

void GeometryNode::DrawUsingSpotLight()
{
    // get the world transformation (hierarchically)
    glm::mat4x4& M = GetTransform();
    // get the view transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& V = m_root->GetViewMat();
    // get the projection transformation (NOTE: normally, this should come from a camera node)
    glm::mat4x4& P = m_root->GetProjectionMat();

    // get the spotlight parameters from the root node
    SpotLight* light = m_root->GetActiveSpotlight();
    // exit if we have not set it
    if (light == nullptr) return;

    // get the shader from the root node
    SpotLightShader* shader = m_root->GetSpotlightShader();

    // set the shader active
    glUseProgram(shader->program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(shader->uniform_m, 1, false, &M[0][0]);
    glUniformMatrix4fv(shader->uniform_v, 1, false, &V[0][0]);
    glUniformMatrix4fv(shader->uniform_p, 1, false, &P[0][0]);

    // LIGHT CALCULATIONS
    // we need

    // get the normal matrix which transforms normals from OCS to ECS and pass it to the shader
    glm::mat4x4 normal_matrix = glm::inverse(glm::transpose(V * M));
    glUniformMatrix4fv(shader->uniform_normal_matrix_ecs, 1, false, &normal_matrix[0][0]);

    // the light position and target are transformed to ECS coordinates
    glm::vec4 light_position_ecs = V * glm::vec4(light->m_transformed_position, 1.0f);
    glm::vec4 light_target_ecs = V * glm::vec4(light->m_transformed_target, 1.0f);

    // get the direction from the light position to the target (target - position)
    glm::vec4 light_direction_ecs = glm::normalize(light_target_ecs - light_position_ecs);

    // pass them to the shader as uniform vec3
    glUniform3f(shader->uniform_light_position_ecs, light_position_ecs.x, light_position_ecs.y, light_position_ecs.z);
    glUniform3f(shader->uniform_light_direction_ecs, light_direction_ecs.x, light_direction_ecs.y, light_direction_ecs.z);

    // the light color is passed as a uniform vec3
    glUniform3f(shader->uniform_light_color, light->m_color.x, light->m_color.y, light->m_color.z);

    // pass the light matrix
    glm::mat4x4 light_view_projection_matrix = m_root->GetLightProjectionMat() * m_root->GetLightViewMat();
    glUniformMatrix4fv(shader->uniform_light_view_projection, 1, false, &light_view_projection_matrix[0][0]);

    // pass the inverse view matrix
    glm::mat4x4 view_inverse_matrix = glm::inverse(V);
    glUniformMatrix4fv(shader->uniform_view_inverse, 1, false, &view_inverse_matrix[0][0]);

    // pass the shadow map sampler and texture
    // activate texture unit 4 for the shadow map texture
    // we can do this here instead of the for loop since
    // this does not change within the for loop
    glActiveTexture(GL_TEXTURE4);
    // bind the depth texture to the active texture unit
    glBindTexture(GL_TEXTURE_2D, light->m_shadow_map_texture_id);
    // send the sampler to the shader
    glUniform1i(shader->uniform_sampler_shadow_map, 4);

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

        // load textures
        // On the C++ side, this process requires two steps for each texture
        // First: activate a texture unit and bind the texture to that texture unit
        // Second: pass a uniform to the shader for each texture to be used as a "sampler".
        // On the GPU side, you just need to access the texture by using the sampler uniform you created in the C++ side

        // check if diffuse texture is present
        if (!cur_material.m_diffuse_opacity_tex_file.empty())
        {
            // activate texture unit 0 for the diffuse texture
            glActiveTexture(GL_TEXTURE0);
            // bind the diffuse texture to the active texture unit
            // the first parameter is the target. for 2D textures we use GL_TEXTURE_2D
            // the second parameter is the value that was returned from glGenTextures
            glBindTexture(GL_TEXTURE_2D, cur_material.m_diffuse_opacity_tex->get_texture_gl_id());
        }

        // check if normal texture is present
        if (!cur_material.m_normal_tex_file.empty())
        {
            // activate texture unit 1 for the normal texture
            glActiveTexture(GL_TEXTURE1);
            // bind the normal texture to the active texture unit
            // the first parameter is the target. for 2D textures we use GL_TEXTURE_2D
            // the second parameter is the value that was returned from glGenTextures
            glBindTexture(GL_TEXTURE_2D, cur_material.m_normal_tex->get_texture_gl_id());
        }

        // check if specular texture is present
        if (!cur_material.m_specular_gloss_tex_file.empty())
        {
            // activate texture unit 2 for the specular texture
            glActiveTexture(GL_TEXTURE2);
            // bind the specular texture to the active texture unit
            // the first parameter is the target. for 2D textures we use GL_TEXTURE_2D
            // the second parameter is the value that was returned from glGenTextures
            glBindTexture(GL_TEXTURE_2D, cur_material.m_specular_gloss_tex->get_texture_gl_id());
        }

        // check if specular texture is present
        if (!cur_material.m_emission_tex_file.empty())
        {
            // activate texture unit 3 for the emission texture
            glActiveTexture(GL_TEXTURE3);
            // bind the emission texture to the active texture unit
            // the first parameter is the target. for 2D textures we use GL_TEXTURE_2D
            // the second parameter is the value that was returned from glGenTextures
            glBindTexture(GL_TEXTURE_2D, cur_material.m_emission_tex->get_texture_gl_id());
        }

        // now send the samplers as uniforms
        // sampler 0 is the diffuse texture, 1 is the normal texture, etc (same as above)
        glUniform1i(shader->uniform_sampler_diffuse, 0);
        glUniform1i(shader->uniform_sampler_normal, 1);
        glUniform1i(shader->uniform_sampler_specular, 2);
        glUniform1i(shader->uniform_sampler_emission, 3);

        // also pass parameters to check within the shader if a texture exists
        // this is important because if we do not check this, the glsl function "texture" will return
        // the value vec3(0,0,0) if a texture does not exist, causing the whole object to be black
        glUniform1i(shader->uniform_has_sampler_diffuse, cur_material.m_diffuse_opacity_tex_loaded);
        glUniform1i(shader->uniform_has_sampler_normal, cur_material.m_normal_tex_loaded);
        glUniform1i(shader->uniform_has_sampler_specular, cur_material.m_specular_gloss_tex_loaded);
        glUniform1i(shader->uniform_has_sampler_emission, cur_material.m_emission_tex_loaded);

        // draw within a range in the index buffer
        glDrawRangeElements(
            GL_TRIANGLES,
            m_ogl_mesh->elements[i].start_index,
            m_ogl_mesh->elements[i].start_index+m_ogl_mesh->elements[i].triangles*3,
            m_ogl_mesh->elements[i].triangles*3,
            GL_UNSIGNED_INT,
            (void*)(m_ogl_mesh->elements[i].start_index*sizeof(GLuint))
            );

        // set the texture units to not point to any textures
        // if we do not do this, then the texture units will point to the bound textures
        // until we set them again
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

// rendering to a depth buffer in the shadow map
// does not need to have any color or texture information
// so the DrawToShadowMap function is quite simple
// we do not need to pass any material or texture information
// to the shadow map shader
// we only need to pass the geometry data and the necessary
// transformations
void GeometryNode::DrawToShadowMap()
{
    // get the world transformation (hierarchically)
    glm::mat4x4& M = GetTransform();
    // get the light view transformation (NOTE: normally, this should come from a light node)
    glm::mat4x4& LV = m_root->GetLightViewMat();
    // get the light projection transformation (NOTE: normally, this should come from a light node)
    glm::mat4x4& LP = m_root->GetLightProjectionMat();

    // this transforms OCS->WCS->LCS->CSS;
    glm::mat4x4 mvp = LP * LV * M;

    // get the shader from the root node
    ShadowMapShader* shader = m_root->GetShadowMapShader();

    // set the shader active
    glUseProgram(shader->program_id);

    // pass any global shader parameters (independent of material attributes)
    glUniformMatrix4fv(shader->uniform_mvp, 1, false, &mvp[0][0]);

    // bind the VAO
    glBindVertexArray(m_ogl_mesh->vao);

    // loop through all the elements
    for (GLint i=0; i < m_ogl_mesh->num_elements; i++)
    {
        if (m_ogl_mesh->elements[i].triangles==0)
            continue;

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