#version 330 core
//----------------------------------------------------//
//                                                    //
// File: OmniLight.vert				                  //
// Vertex shader code for the OmniLight shader		  //
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

uniform mat4 uniform_normal_matrix_ecs;

// the normal that is passed to the fragment shader
out vec3 normal_ecs_v;

// the position that is passed to the fragment shader
out vec3 position_ecs_v;

// for texture mapping we also need to pass the texture coordinates
// to the fragment shader
out vec2 texcoord;

void main(void)
{
// transform the vertex normal with the normal matrix
// we can do this in the fragment shader but since this is a per-vertex evaluation
// so we do it in the vertex shader to save instructions
	normal_ecs_v = vec3(uniform_normal_matrix_ecs * vec4(normal, 0.0)).xyz;

// for shading from omni lights, we also need the current vertex in the fragment shader
	position_ecs_v = vec3(uniform_v * uniform_m * vec4(position, 1.0)).xyz;

// pass the texture coordinates
	texcoord = texcoord0;

// vertex position in CSS
	gl_Position = uniform_p * vec4(position_ecs_v, 1);
}
