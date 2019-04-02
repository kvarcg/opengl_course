//----------------------------------------------------//
//                                                    //
// File: OGLMaterial.cpp                              //
// OGLMaterial holds the reflectance properties for   //
// the loaded models                                  //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

// includes ////////////////////////////////////////
#include "HelpLib.h"        // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "OBJMaterial.h"    // - Header file for the OBJMaterial class

// Constructor
OBJMaterial::OBJMaterial(void):
m_diffuse(1),
m_specular(0),
m_emission(0),
m_opacity(1),
m_gloss(0),
m_diffuse_opacity_tex_file(),
m_specular_gloss_tex_file(),
m_emission_tex_file(),
m_normal_tex_file(),
m_name()
{

}

// Destructor
OBJMaterial::~OBJMaterial()
{

}

// other functions
bool OBJMaterial::operator == (OBJMaterial& m)
{
    return (
        m_diffuse                   ==        m.m_diffuse                   &&
        m_specular                  ==        m.m_specular                  &&
        m_emission                  ==        m.m_emission                  &&
        m_gloss                     ==        m.m_gloss                     &&
        m_opacity                   ==        m.m_opacity                   &&
        m_diffuse_opacity_tex_file  ==        m.m_diffuse_opacity_tex_file  &&
        m_specular_gloss_tex_file   ==        m.m_specular_gloss_tex_file   &&
        m_emission_tex_file         ==        m.m_emission_tex_file         &&
        m_normal_tex_file           ==        m.m_normal_tex_file           &&
        m_name                      ==        m.m_name
        );
}

void OBJMaterial::dump()
{
    PrintToOutputWindow("\tMaterial %s info:", m_name.c_str());
    PrintToOutputWindow("\t\tdiffuse  : % f % f % f", m_diffuse [0], m_diffuse [1], m_diffuse [2]);
    PrintToOutputWindow("\t\tspecular: % f % f % f", m_specular [0], m_specular [1], m_specular [2]);
    PrintToOutputWindow("\t\temission : % f % f % f", m_emission[0], m_emission[1], m_emission[2]);
    PrintToOutputWindow("\t\tgloss: %f \topacity : % f", m_gloss, m_opacity);

    if (!m_diffuse_opacity_tex_file.empty())
        PrintToOutputWindow("\t\tdiffuse_opacity_tex_file: %s\n", m_diffuse_opacity_tex_file.c_str());
    if (!m_specular_gloss_tex_file.empty())
        PrintToOutputWindow("\t\tspecular_gloss_tex_file: %s\n", m_specular_gloss_tex_file.c_str());
    if (!m_normal_tex_file.empty())
        PrintToOutputWindow("\t\tnormal_tex_file: %s\n", m_normal_tex_file.c_str());
    if (!m_emission_tex_file.empty())
        PrintToOutputWindow("\t\temission_tex_file: %s\n", m_emission_tex_file.c_str());
}

// eof ///////////////////////////////// class material