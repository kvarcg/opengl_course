#version 330 core
//----------------------------------------------------//
//                                                    //
// File: BasicGeometry.frag                           //
// Fragment shader code for the BasicGeometry shader  //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)					  //
//                                                    //
//----------------------------------------------------//

layout(location = 0) out vec4 out_color;

// the incoming texture coordinates from the vertex shader
in vec2 texcoord;

// the material color
uniform vec4 uniform_material_color;

// samplers
uniform sampler2D uniform_sampler_diffuse;
uniform int uniform_has_sampler_diffuse;

// the ambient light color
uniform vec4 uniform_ambient_light_color;

void main(void)
{
	// get the diffuse for this fragment based on the interpolated uv coordinates
	vec4 diffuse_tex = uniform_material_color;
	if (uniform_has_sampler_diffuse > 0)
	{
		diffuse_tex = diffuse_tex * texture(uniform_sampler_diffuse, texcoord.xy);
		// alpha testing
		if (diffuse_tex.a < 1.0) 
		{
			discard;
		}
	}

	// final color
	out_color = uniform_ambient_light_color * diffuse_tex;
}