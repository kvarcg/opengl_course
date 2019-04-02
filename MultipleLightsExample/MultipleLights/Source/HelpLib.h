//----------------------------------------------------//
//                                                    //
// File: HelpLib.h                                    //
// HelpLib contains some functions necessary for      //
// simplifying basic operations (writing to output    //
// window, basic OpenGL error checking, etc.)         //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

// Windows API and system calls
#include <windows.h>
// C++ Standard Input and Output Library
#include <stdio.h>
// C++ String
#include <string>
// C++ Vector
#include <vector>
// The OpenGL Extension Library
#include <glew/glew.h>
// The FREEGLUT Library for the user interface and windows
#include <gl/freeglut.h>

// ---------------- import GLM ---------------------------
#pragma warning( disable : 4244 )
#define GLM_PRECISION_MEDIUMP_FLOAT

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/extented_min_max.hpp>
#include <glm/gtx/simd_mat4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/epsilon.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef GLM_PRECISION_HIGHP_FLOAT
typedef glm::highp_float_t flt;
#else
typedef glm::mediump_float_t flt;
#endif

//----------------- declarations ------------------

// include GLEW libraries
#pragma comment (lib, "glew32.lib")

// useful preprocessor defines
#define NULL_TERMINATED_CHAR  '\0'
#define NEW_LINE_CHAR         '\n'
#define SAFE_FREE(_x)         { if ((_x) != nullptr) { free    ((_x)); _x = nullptr; } }
#define SAFE_DELETE(_x)       { if ((_x) != nullptr) { delete   (_x);  _x = nullptr; } }
#define SAFE_DELETE_ARRAY_POINTER(_x) { if ((_x) != nullptr) { delete[] (_x);  _x = nullptr; } }

#define SAFE_DELETE_ARRAY(_x, _s) for (int i = 0; i < _s; ++i) { \
    if ((_x[i]) != nullptr) { delete   (_x[i]);  _x[i] = nullptr; }}

// declarations for Library functions

// Print to VS output window
bool PrintToOutputWindow(const char * fmt, ...);

// Check for OpenGL errors
bool glError(void);

// Check for FBO errors
bool checkFrameBufferError(const char* str);

// read a file
bool readFile(std::string& filename, char** data);