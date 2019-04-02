#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicLighting.vert                           //
// Vertex shader code for the BasicLighting shader    //
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
// attribute at index 0 is the incoming vertex position as a vec3
layout(location = 0) in vec3 position;
// attribute at index 1 is the incoming normal color as a vec3
layout(location = 1) in vec3 normal;

// optional input of a vertex shader
// SYNTAX: uniform type variable_name
// the command below is the MVP matrix we passed using glUniformMatrix4fv
uniform mat4 uniform_mvp;

out vec3 normal_ocs;

// Everything happens inside the main function
void main(void)
{
	normal_ocs = normal;

// Vertex shader REQUIRED output:  
// gl_Position is the STANDARD output of the vertex shader
	gl_Position = uniform_mvp * vec4(position, 1.0);
}
