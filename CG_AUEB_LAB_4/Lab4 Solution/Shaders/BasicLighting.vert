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
uniform float uniform_time;

// get a random number between -0.05,0.05
float rand(vec3 n)
{
	// get a random number between 0,1
	float random_number = fract(dot(n.xyz, vec3(17.532, 59.124, 67.354) * 12345.893));
	// get it in the range -0.05,0.05
	return 0.05 * (2.0 * random_number - 1.0);
}

out vec3 vnormal_ecs;

void main(void)
{
	// modify the vertex position by offsetting it in the direction of the normal
	// based on the sine function of a time parameter which returns values in the range of -1,1
	// and a random number which is different for each vertex (based on the normal)
	vec3 extrude_pos = position + normal * sin(uniform_time) * rand(normal);
	
	mat4 normal_matrix = transpose(inverse(uniform_v * uniform_m));
	vnormal_ecs = vec3(normal_matrix * vec4(normal, 0.0)).xyz;

	gl_Position = uniform_p * uniform_v * uniform_m * vec4(extrude_pos,1);
}
