#version 330 core
//----------------------------------------------------//
//                                                    //
// File: DirectionalLight.frag                        //
// Fragment shader code for the DirectionalLight	  //
// shader											  //
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


// the light color
uniform vec3 uniform_light_color;

// the light direction (directional lights have no position)
uniform vec3 uniform_light_direction_ecs;

// the incoming normal in ECS from the vertex shader
in vec3 normal_ecs_v;

void main(void)
{
	// get the incoming transformed ECS normal from the vertex shader
	// due to fragment interpolation, the normal needs to be renormalized
	vec3 normal_ecs = normalize(normal_ecs_v);

	// get the dot product between the light direction and the normal (both in ECS)
	// if the dot product is negative, then the light comes from below the surface
	// in this case, the light does not illuminate the surface at all and we
	// clamp the result to zero
	float ndotl = max(0.0, dot(normal_ecs.xyz, uniform_light_direction_ecs));

	// the final shading equation for diffuse surfaces is
	vec3 diffuse_color = uniform_material_color.xyz * uniform_light_color * ndotl;

	// set the lighting result to be the final color of the fragment
	// the alpha value comes from the material
	out_color = vec4(diffuse_color, uniform_material_color.a);
}
