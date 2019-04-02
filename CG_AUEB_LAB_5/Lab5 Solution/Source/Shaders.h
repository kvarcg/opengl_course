//----------------------------------------------------//
//                                                    //
// File: Shaders.h                                    //
// Shaders can go here                                //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

#ifndef SHADERS_H
#define SHADERS_H

#pragma once
#include "ShaderGLSL.h"

// Shaders
// abstract shader class
struct Shader {};
// omni light shader
class OmniLightShader : Shader
{
public:
    ShaderGLSL*    shader;
    GLint program_id;
    GLint uniform_m;
    GLint uniform_v;
    GLint uniform_p;
    GLint uniform_color;
    GLint uniform_normal_matrix_ecs;
    GLint uniform_light_color;
    GLint uniform_light_position_ecs;
};

// directional light shader
class DirectionalLightShader : Shader
{
public:
    ShaderGLSL*    shader;
    GLint program_id;
    GLint uniform_m;
    GLint uniform_v;
    GLint uniform_p;
    GLint uniform_color;
    GLint uniform_normal_matrix_ecs;
    GLint uniform_light_color;
    GLint uniform_light_direction_ecs;
};

// basic geometry shader
class BasicGeometryShader : Shader
{
public:
    ShaderGLSL*    shader;
    GLint program_id;
    GLint uniform_m;
    GLint uniform_v;
    GLint uniform_p;
    GLint uniform_color;
};

#endif //SHADERS_H

// eof ///////////////////////////////// class Light