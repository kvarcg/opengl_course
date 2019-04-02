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
    std::string                    m_name;
    glm::vec3                    m_color;
    glm::vec3                    m_initial_target;
    glm::vec3                    m_transformed_target;
    glm::vec3                    m_initial_position;
    glm::vec3                    m_transformed_position;

    SpotLight():
        m_initial_target(0,0,0),
        m_initial_position(0,10,0),
        m_color(1,1,1)
    {

    }
};

#endif //LIGHT_H

// eof ///////////////////////////////// class Light