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
 
    SpotLight*                          m_spotlight;

    BasicGeometryShader*                m_basic_geometry_shader;
    SpotLightShader*                    m_spotlight_shader;
    AmbientLightShader*                 m_ambient_light_shader;

    glm::vec3                           m_ambient_light_color;

    glm::mat4x4                         m_light_view_mat;
    glm::mat4x4                         m_light_projection_mat;

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
    void                                SetActiveSpotlight(SpotLight* light)            {m_spotlight = light;}
    void                                SetLightViewMat(glm::mat4x4& mat)               {m_light_view_mat = mat;}
    void                                SetLightProjectionMat(glm::mat4x4& mat)         {m_light_projection_mat = mat;}
    void                                SetAmbientLightColor(glm::vec3& c)              {m_ambient_light_color = c;}

    // get light functions
    SpotLight*                          GetActiveSpotlight(void)                        {return m_spotlight;}
    glm::mat4x4&                        GetLightViewMat(void)                           {return m_light_view_mat;}
    glm::mat4x4&                        GetLightProjectionMat(void)                     {return m_light_projection_mat;}
    glm::vec3&                          GetAmbientLightColor(void)                      {return m_ambient_light_color;}

    // set shader functions
    void                                SetNoLightingShader(BasicGeometryShader* shader) {m_basic_geometry_shader = shader;}
    void                                SetSpotlightShader(SpotLightShader* shader)      {m_spotlight_shader = shader;}
    void                                SetAmbientLightShader(AmbientLightShader* shader){m_ambient_light_shader = shader;}

    // get shader functions
    BasicGeometryShader*                GetNoLightingShader(void)                       {return m_basic_geometry_shader;}
    SpotLightShader*                    GetSpotlightShader(void)                        {return m_spotlight_shader;}
    AmbientLightShader*                 GetAmbientLightShader(void)                     {return m_ambient_light_shader;}
};

#endif //ROOT_H

// eof ///////////////////////////////// class Root