//----------------------------------------------------//
//                                                    //
// File: OmniLight.frag                               //
// Fragment shader code for the OmniLight shader      //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided for the tutorials as part //
// of the BSc course of Computer Graphics at the      //
// Athens University of Economics and Business (AUEB) //
//                                                    //
//----------------------------------------------------//

#version 330 core

layout(location = 0) out vec4 out_color;

// the material color
uniform vec4 uniform_material_color;

// the light color
uniform vec3 uniform_light_color;

// the light position (omni lights have no direction)
uniform vec3 uniform_light_position_ecs;

// the incoming normal in ECS from the vertex shader
in vec3 normal_ecs_v;

// the incoming vertex position in ECS from the vertex shader
in vec3 position_ecs_v;

// the incoming texture coordinates from the vertex shader
in vec2 texcoord;

// samplers
// a sampler is used to retrieve information from a texture
// practically, it points to the texture
uniform sampler2D uniform_sampler_diffuse;

// a simple uniform (0,1) to indicate if a diffuse texture is present
uniform int uniform_has_sampler_diffuse;

void main(void)
{
	vec4 diffuse_tex = uniform_material_color;

	// get the diffuse value for this fragment based on the interpolated uv coordinates
	// that we got from the vertex shader
	// the texture GLSL function retrieves the texture value
	// it accepts as parameters the sampler and the texture coordinates
	// the retrieved value can be either filtered or unfiltered
	// this is based on the texture parameters that were used when
	// the texture was generated (e.g. if mipmaps were generated, or if
	// nearest neighbour filtering was used)	
	vec4 texture_value = texture(uniform_sampler_diffuse, texcoord.xy);

	// alpha testing
	// if the alpha value is below a threshold then skip drawing the pixel
	// discard does just this, is similar to return in C++
	if (texture_value.a < 1.0)
	{
		discard;
	}

	// if a diffuse texture is present, multiply the material color with the texture value
	if (uniform_has_sampler_diffuse > 0)
	{
		diffuse_tex *= texture_value;
	}

	// get the incoming transformed ECS normal from the vertex shader
	// due to fragment interpolation, the normal needs to be renormalized
	vec3 normal_ecs = normalize(normal_ecs_v);

	// get a vector from the vertex position to the light position
	vec3 vertex_to_light_ecs = uniform_light_position_ecs - position_ecs_v;

	// get the distance between the vertex and the light
	// this is the same as checking the length of the vertex to light vector
	float dist_to_light = length(vertex_to_light_ecs);

	// now, normalize the vector (for the lighting calculations)
	vertex_to_light_ecs = normalize(vertex_to_light_ecs);

	// get the dot product between the vertex-to-light direction and the normal (both in ECS)
	// if the dot product is negative, then the light comes from below the surface
	// in this case, the light does not illuminate the surface at all and we
	// clamp the result to zero
	float ndotl = max(0.0, dot(normal_ecs.xyz, vertex_to_light_ecs));

	// get the attenuation factor required for point lights
	// no attenuation
	float attenuation = 1.0;
	float c0 = 1.0;
	float c1 = 0.01;
	float c2 = 0.003;
	// linear attenuation
	//attenuation = 1.0 / (c0 + c1 * dist_to_light);
	// quadratic attenuation
	//attenuation = 1.0 / (c0 + c1 * dist_to_light + c2 * dist_to_light * dist_to_light);

	// the final shading equation for diffuse surfaces is
	vec3 diffuse_color = diffuse_tex.rgb * uniform_light_color * ndotl * attenuation;

	vec3 ambient_color = vec3(0.02, 0.1, 0.15);

	// set the lighting result to be the final color of the fragment
	// the alpha value comes from the material
	out_color = vec4(ambient_color + diffuse_color, uniform_material_color.a);
}
