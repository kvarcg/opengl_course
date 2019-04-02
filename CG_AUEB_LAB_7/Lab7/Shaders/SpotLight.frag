#version 330 core
//----------------------------------------------------//
//                                                    //
// File: SpotLight.frag                               //
// Fragment shader code for the SpotLight shader      //
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

// the light position
uniform vec3 uniform_light_position_ecs;

// the light direction
uniform vec3 uniform_light_direction_ecs;

// the incoming normal in ECS from the vertex shader
in vec3 normal_ecs_v;

// the incoming vertex position in ECS from the vertex shader
in vec3 position_ecs_v;

// the incoming texture coordinates from the vertex shader
in vec2 texcoord;

// samplers
uniform sampler2D uniform_sampler_diffuse;
uniform int uniform_has_sampler_diffuse;
uniform sampler2D uniform_sampler_shadow_map;

// the camera view inverse and light view projection matrices are needed
// to transform the current vertex position from camera eye space to light projective space
uniform mat4 uniform_view_inverse;
uniform mat4 uniform_light_view_projection;

float is_point_in_shadow()
{
// to check if a point is in shadow we need to
// check if the current fragment's position is hidden in the shadow map (something else is occluding it)
// to sample the shadow map we need to transform the incoming vertex position which is in camera eye space to 
// the light projective space

// transform the ECS to WCS by using the inverse camera view matrix
	vec4 position_wcs = uniform_view_inverse * vec4(position_ecs_v, 1);
// transform the WCS to light projective space
	vec4 position_lcs = uniform_light_view_projection * position_wcs;
// divide by w. Everytime a projection matrix is applied, the resulting
// coordinates are in affine space. Perspective divide is required to convert it to linear space
	position_lcs /= position_lcs.w;
// to sample the shadow map we need the XY coordinates of the light projection coordinates
// however, the projection matrix multiplication returns values in the range [-1,1]
// to access a textures we need to convert them to 0-1 range
// the following first adds 1 to the range to get it to [0,2] range
// and then a multiplication by 0.5 gets it to the [0,1] range
	position_lcs.xyz = (position_lcs.xyz + 1) * 0.5;

// check that we are inside light clipping frustum
// currently we are in the [0,1] range. if we exceed it (we are outside of what the shadow map sees) we need to know 
// what this area will be. Lit or in shadow. Returning 0 sets it to be in shadow
if (position_lcs.x < 0.0 || position_lcs.y < 0.0 || position_lcs.x > 1.0 || position_lcs.y > 1.0) return 0.0;
// in general, conditional statements are BAD for GPUs. A more optimized version of the above code is shown below
// if ((clamp(position_lcs.xy, vec2(0,0), vec2(1,1)) - position_lcs.xy) != vec2(0,0)) return 1.0;

// the XY values of the position_lcs variable can now be used to check the shadow map
// we sample the texture in the XY axis and get back a value in the Z axis for that point (in the range 0-1)
// that point represents the Z value of the visible surface as viewed from the light
	float shadow_map_z = texture(uniform_sampler_shadow_map, position_lcs.xy).r;

// SHADOW CHECK
// + shaded -> 0.0f
// - lit -> 1.0f
// now we simply need to check if the z value of the shadow map is smaller (closer to the light) than
// the z value of the point in the current fragment
// in that case the point is in shadow and we set the isInShadow to 0.
// this is later multiplied with the result of the shading equation
// setting the current fragment color black (shadowed)
// the value of 0.0005 is the value of a constant bias (offset) that is applied to the calculations
// to avoid self-shadowing the surface
	float constant_depth_bias = 0.0005;
	float isInShadow = 1.0;
	
	if (position_lcs.z > shadow_map_z + constant_depth_bias)
		isInShadow = 0.0;
	
	return isInShadow;
}

float is_point_in_shadow_pcf_5()
{
	// these are the same as before
	vec4 position_wcs = uniform_view_inverse * vec4(position_ecs_v, 1);
	vec4 position_lcs = uniform_light_view_projection * position_wcs;
	position_lcs /= position_lcs.w;
	position_lcs.xyz = (position_lcs.xyz + 1) * 0.5;
	float constant_depth_bias = 0.0005;
	if ((clamp(position_lcs.xy, vec2(0,0), vec2(1,1)) - position_lcs.xy) != vec2(0,0)) return 0.0;

	/*
	Implement PCF here
	*/
	
	// PCF
	// 4 sample per pixel fixed kernel
	// + shaded -> 0.0f 
	// - lit -> 1.0f
	// do PCF only if point is in shadow
	// set a shadow map offset (two texels)
	// The value of 0 is for the lod level. We have no mipmaps, so this is 0.
	// See here http://www.opengl.org/sdk/docs/manglsl/xhtml/textureSize.xml
	float shadow_map_step = 2.0/float(textureSize(uniform_sampler_shadow_map, 0));


	// return that the point is visible for now
	// this will need to change
	return 1.0;
}

float is_point_in_shadow_pcf_9()
{
	// these are the same as before
	vec4 position_wcs = uniform_view_inverse * vec4(position_ecs_v, 1);
	vec4 position_lcs = uniform_light_view_projection * position_wcs;
	position_lcs /= position_lcs.w;
	position_lcs.xyz = (position_lcs.xyz + 1) * 0.5;
	float constant_depth_bias = 0.0005;
	if ((clamp(position_lcs.xy, vec2(0,0), vec2(1,1)) - position_lcs.xy) != vec2(0,0)) return 0.0;

	/*
	Implement PCF here
	*/
	
	// PCF
	// 9 sample per pixel fixed kernel
	// + shaded -> 0.0f 
	// - lit -> 1.0f
	// do PCF only if point is in shadow
	// set a shadow map offset (two texels)
	// The value of 0 is for the lod level. We have no mipmaps, so this is 0.
	// See here http://www.opengl.org/sdk/docs/manglsl/xhtml/textureSize.xml
	float shadow_map_step = 2.0/float(textureSize(uniform_sampler_shadow_map, 0));

	// return that the point is visible for now
	// this will need to change
	return 1.0;
}


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
	// this is important because if we do not check this, the GLSL function "texture" will return
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

	// for spotlights, check if the angle between light direction and the angle-to-the-vertex is 
	// less than a prespecified angle. in this example, cut everything over 90 degrees
	float spotlight_value = max(0.0, dot(-vertex_to_light_ecs, uniform_light_direction_ecs));

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
	attenuation = 1.0 / (c0 + c1 * dist_to_light);
	// quadratic attenuation
	//attenuation = 1.0 / (c0 + c1 * dist_to_light + c2 * dist_to_light * dist_to_light);

	// the final shading equation for diffuse surfaces is
	vec3 diffuse_color = diffuse_tex.rgb * uniform_light_color * ndotl * attenuation * spotlight_value;

	// add shadows
	float shadow_factor = 1.0;
	shadow_factor = is_point_in_shadow();
	//shadow_factor = is_point_in_shadow_pcf_5();
	//shadow_factor = is_point_in_shadow_pcf_9();
	diffuse_color *= shadow_factor;

	vec3 ambient_color = vec3(0.02, 0.1, 0.15) * diffuse_tex.rgb;

	// set the lighting result to be the final color of the fragment
	// the alpha value comes from the material
	out_color = vec4(ambient_color + diffuse_color, uniform_material_color.a);
}
