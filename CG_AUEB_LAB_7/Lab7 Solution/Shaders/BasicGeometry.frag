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

// the material color
uniform vec4 uniform_material_color;

// the incoming texture coordinates from the vertex shader
in vec2 texcoord;

// samplers
uniform sampler2D uniform_sampler_diffuse;
uniform int uniform_has_sampler_diffuse;

void main(void)
{
	// get the diffuse for this fragment based on the interpolated uv coordinates
	// that we got from the vertex shader
	// the texture GLSL function retrieves the texture value
	// it accepts as parameters the sampler and the texture coordinates
	// the retrieved value can be either filtered or unfiltered
	// this is based on the texture parameters that were used when
	// the texture was generated (e.g. if mipmaps were generated, or if
	// nearest neighbour filtering was used)
	// first we check if the texture exists
	// this is important because if we do not check this, the glsl function "texture" will return
	// the value vec3(0,0,0) if a texture does not exist, causing the whole object to be black
	vec4 diffuse_tex = uniform_material_color;
	if (uniform_has_sampler_diffuse > 0)
	{
		diffuse_tex = diffuse_tex * texture(uniform_sampler_diffuse, texcoord.xy);
		// alpha testing
		// if the alpha value is below a threshold then skip drawing the pixel
		// discard does just this, is similar to return in C++
		if (diffuse_tex.a < 1.0) discard;
	}

	// final color
	out_color = diffuse_tex;
}