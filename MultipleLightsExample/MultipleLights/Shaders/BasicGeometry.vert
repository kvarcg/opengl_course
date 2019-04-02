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

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord0;
layout(location = 3) in vec2 texcoord1;
layout(location = 4) in vec3 tangent;

uniform mat4 uniform_m;
uniform mat4 uniform_v;
uniform mat4 uniform_p;

// for texture mapping we also need to pass the texture coordinates
// to the fragment shader
out vec2 texcoord;

void main(void)
{
// pass the texture coordinates
	texcoord = texcoord0;

	gl_Position = uniform_p * uniform_v * uniform_m * vec4(position,1);
}