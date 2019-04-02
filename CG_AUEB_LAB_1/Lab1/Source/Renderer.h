//----------------------------------------------------//
//                                                    //
// File: Renderer.h                                   //
// Initialization, rendering and input handling       //
// happens here                                       //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//


// This function is called before GLUT goes into its main loop.
bool InitializeRenderer(void);

// This function draws everything in the OpenGL window
void Render(void);

// When a resize event occurs, this function is called
void Resize(int width, int height);

// This function handles the presses of "character keys"
void Keyboard(unsigned char key, int x, int y);

// This function handles special keys, like arrow or function keys
void KeyboardSpecial(int key, int x, int y);

// Release all memory allocated by pointers using new
// Also need to delete any gluQuadrics
void ReleaseGLUT();