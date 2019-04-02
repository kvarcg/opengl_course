#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicLighting.frag                           //
// Fragment shader code for the BasicLighting shader  //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided for the tutorials as part //
// of the BSc course of Computer Graphics at the      //
// Athens University of Economics and Business (AUEB) //
//                                                    //
//----------------------------------------------------//

layout(location = 0) out vec4 out_color;

// the material color
uniform vec4 uniform_material_color;

in vec3 vnormal_ecs;

void main(void)
{
	vec4 normal_ecs = vec4(normalize(vnormal_ecs), 0);

	float light = max(0.0, dot(normal_ecs.xyz, vec3(0,0,1)));

	// final color
	out_color = uniform_material_color * light;
}
