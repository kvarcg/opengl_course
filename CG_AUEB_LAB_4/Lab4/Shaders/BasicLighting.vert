#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicLighting.vert                           //
// Vertex shader code for the BasicLighting shader    //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided for the tutorials as part //
// of the BSc course of Computer Graphics at the      //
// Athens University of Economics and Business (AUEB) //
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

out vec3 vnormal_ecs;

void main(void)
{
	mat4 normal_matrix = transpose(inverse(uniform_v * uniform_m));
	vnormal_ecs = vec3(normal_matrix * vec4(normal, 0.0)).xyz;

	gl_Position = uniform_p * uniform_v * uniform_m * vec4(position,1);
}
