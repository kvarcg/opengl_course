//----------------------------------------------------//
//                                                    //
// File: Light.h                                      //
// Omni and directional lights are defined here       //
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

// Omni directional lights are lights with a constant intensity value that does
// not vary with direction
// Mainly used for simulating light sources such as a candle, etc.
// Default position is 10 units at the Y axis in WCS and default color is white
// we are using an initial and a transformed position to set an initial world position
// and then apply transformations (e.g. rotations) to do light animation
struct OmniLight
{
    std::string                  m_name;
    glm::vec3                    m_color;
    glm::vec4                    m_initial_position;
    glm::vec4                    m_tranformed_position;

    OmniLight():
        m_initial_position(0,10,0,1),
        m_color(1,1,1)
    {

    }
};

// Directional lights are lights that are located so far away compared to the world
// that their direction does not change along the entire surface
// Mainly used for simulating light sources such as the sun
// Default direction is looking down the Y axis in WCS and default color is white
// we are using an initial and a transformed direction to set an initial world direction
// and then apply transformations (e.g. rotations) to do light animation
struct DirectionalLight
{
    std::string                  m_name;
    glm::vec3                    m_color;
    glm::vec4                    m_initial_direction;
    glm::vec4                    m_transformed_direction;

    DirectionalLight():
        m_initial_direction(0,-1,0,0),
        m_color(1,1,1)
    {

    }
};

#endif //LIGHT_H

// eof ///////////////////////////////// class Light