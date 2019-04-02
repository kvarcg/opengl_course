#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicGeometry.frag                           //
// Fragment shader code for the BasicGeometry shader  //
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

// optional input of a fragment shader
// SYNTAX: in type variable_name
// the command below defines a vec4 coming from the previous stages
// This is the vertex_color variable we passed from the vertex shader
in vec4 vertex_color;

uniform vec4 uniform_color;

// Everything happens inside the main function
void main(void)
{
// Fragment shader REQUIRED output:
// write to the output variable, the incoming vertex color
out_color = vertex_color;
}