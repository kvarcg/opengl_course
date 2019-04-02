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

// Everything happens inside the main function
void main(void)
{
	// final color
	out_color = uniform_material_color;
}