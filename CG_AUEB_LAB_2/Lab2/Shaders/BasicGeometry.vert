#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicGeometry.vert                           //
// Vertex shader code for the BasicGeometry shader    //
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

// Remember: a vertex shader receives vertices as input and sends vertices as output
// Typical use of a pass-through vertex shader:
// Transform incoming vertices from object space to post projective space
// More information: http://www.opengl.org/wiki/Vertex_Shader
//
// Vertex shader REQUIRED input:
// - a vertex data structure
// the structure of the incoming vertex data
// use the layout syntax and the in type qualifier
// SYNTAX: layout(location = #)? in type name
// attribute at index 0 is the incoming vertex position as a vec4
layout(location = 0) in vec4 position;
// attribute at index 1 is the incoming vertex color as a vec4
layout(location = 1) in vec4 color;

// optional input of a vertex shader
// SYNTAX: uniform type variable_name
// the command below is the MVP matrix we passed using glUniformMatrix4fv
uniform mat4 uniform_mvp;

// optional output of a vertex shader
// SYNTAX: out type variable_name
// the command below defines a vec4 which will be passed the fragment shader
out vec4 vertex_color;

// Everything happens inside the main function
void main(void)
{
// pass to the fragment shader the color of this vertex
	vertex_color = color;

// Vertex shader REQUIRED output:  
// gl_Position is the STANDARD output of the vertex shader
	gl_Position = uniform_mvp * position;
}
