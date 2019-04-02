#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicShadow.vert                             //
// Vertex shader code for the BasicShadow shader      //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)					  //
//                                                    //
//----------------------------------------------------//

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord0;
layout(location = 3) in vec2 texcoord1;
layout(location = 4) in vec3 tangent;

uniform mat4 uniform_mvp;

void main(void)
{
	gl_Position = uniform_mvp * vec4(position, 1);
}