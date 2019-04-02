#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicLighting.frag                           //
// Fragment shader code for the BasicLighting shader  //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)					  //
//                                                    //
//----------------------------------------------------//

// #version 330 core means we use core GLSL 3.30

// Remember: a fragment shader receives the rasterized fragments as input and writes values to a framebuffer
// Typical use of a pass-through fragment shader:
// Write a color to each fragment
// More information: http://www.opengl.org/wiki/Fragment_Shader
//
// Fragment shader REQUIRED input:
// - NOTHING: we can write a constant value
// Fragment shader REQUIRED output:
// the structure of the outgoing fragment data
// use the layout syntax and the out type qualifier
// SYNTAX: layout(location = #)? out type name
// attribute at index 0 is the outgoing frament color as a vec4
layout(location = 0) out vec4 out_color;

uniform vec4 uniform_color;

in vec3 normal_ocs;

uniform mat4 uniform_normal_matrix;

// Everything happens inside the main function
void main(void)
{

// transform the incoming normal for OCS to ECS using the normal matrix
vec4 normal_ecs = uniform_normal_matrix * vec4(normal_ocs, 0);
normal_ecs.xyz = normalize(normal_ecs.xyz);

// get the dot product between the normal and the view direction
// (this simulates a light at the position of the camera)
float light = max(0.0, dot(normal_ecs.xyz, vec3(0,0,1)));

// Fragment shader REQUIRED output:
// write to the output variable, the incoming vertex color
out_color = uniform_color * light;
}
