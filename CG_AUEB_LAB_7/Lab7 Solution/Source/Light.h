//----------------------------------------------------//
//                                                    //
// File: Light.h                                      //
// Lights are defined here                            //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

#ifndef LIGHT_H
#define LIGHT_H

#pragma once

// Spotlights are lights which contain a position and a target (target - position specifies the light's direction)
struct SpotLight
{
    std::string                  m_name;
    glm::vec3                    m_color;
    glm::vec3                    m_initial_target;
    glm::vec3                    m_transformed_target;
    glm::vec3                    m_initial_position;
    glm::vec3                    m_transformed_position;

    // these three parameters define the viewing frustom of the light
    // to construct the perspective projection matrix
    // it has the same logic with the way the perspective projection matrix
    // is constructed for the camera (in Resize)
    float                        m_near_range;
    float                        m_far_range;
    float                        m_aperture;

    GLint                        m_shadow_map_texture_id;

    SpotLight():
        m_initial_target(0,0,0),
        m_initial_position(0,10,0),
        m_color(1,1,1),
        m_near_range(1),
        m_far_range(100),
        m_aperture(30)
    {

    }
};

#endif //LIGHT_H

// eof ///////////////////////////////// class Light