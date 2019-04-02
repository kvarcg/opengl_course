//----------------------------------------------------//
//                                                    //
// File: Root.h                                       //
// This scene graph is a basic example for the        //
// object relational management of the scene          //
// This holds the root node                           //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//
#ifndef ROOT_H
#define ROOT_H

#pragma once
//using namespace

// includes ////////////////////////////////////////
#include "GroupNode.h"
#include "../Shaders.h"
#include "../Light.h"

// defines /////////////////////////////////////////


// forward declarations ////////////////////////////


// class declarations //////////////////////////////

class Root : public GroupNode
{
protected:
    // protected variable declarations
    glm::mat4x4                         m_view_mat;
    glm::mat4x4                         m_projection_mat;

    DirectionalLight*                   m_directional_light;
    OmniLight*                          m_omni_light;

    DirectionalLightShader*             m_directional_light_shader;
    OmniLightShader*                    m_omni_light_shader;

    // protected function declarations

private:
    // private variable declarations
    void                                SetRoot(GroupNode* gnd);

    // private function declarations


public:
    // Constructor
    Root(void);

    // Destructor
    ~Root(void);

    // public function declarations
    void                                Init(void);
    void                                Update(void);
    void                                Draw(int shader_type);

    // get functions
    glm::mat4x4&                        GetViewMat(void)                                {return m_view_mat;}
    glm::mat4x4&                        GetProjectionMat(void)                          {return m_projection_mat;}

    // set functions
    void                                SetViewMat(glm::mat4x4& mat)                    {m_view_mat = mat;}
    void                                SetProjectionMat(glm::mat4x4& mat)              {m_projection_mat = mat;}

    // set light functions
    void                                SetActiveDirectionalLight(DirectionalLight* light) {m_directional_light = light;}
    void                                SetActiveOmnilLight(OmniLight* light)           {m_omni_light = light;}

    // get light functions
    DirectionalLight*                    GetActiveDirectionalLight(void)                {return m_directional_light;}
    OmniLight*                           GetActiveOmnilLight(void)                      {return m_omni_light;}

    // set shader functions
    void                                SetDirectionalLightShader(DirectionalLightShader* shader) {m_directional_light_shader = shader;}
    void                                SetOmniLightShader(OmniLightShader* shader)     {m_omni_light_shader = shader;}

    // get shader functions
    DirectionalLightShader*             GetDirectionalLightShader(void)                 {return m_directional_light_shader;}
    OmniLightShader*                    GetOmniLightShader(void)                        {return m_omni_light_shader;}
};

#endif //ROOT_H

// eof ///////////////////////////////// class Root