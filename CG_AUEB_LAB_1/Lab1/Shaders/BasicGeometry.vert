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

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

out vec4 vertex_color;

void main(void)
{
   gl_Position = position;
   vertex_color = color;
}