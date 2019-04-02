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

#version 330 core

layout(location = 0) out vec4 out_color;

// the material color
uniform vec4 uniform_material_color;

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
	//if (texture_value.a < 1.0)
	//{
	//	discard;
	//}

	// if a diffuse texture is present, multiply the material color with the texture value
	if (uniform_has_sampler_diffuse > 0)
	{
		diffuse_tex *= texture_value;
	}

	// final color
	out_color = diffuse_tex;
}