//----------------------------------------------------//
//                                                    //
// File: Main.cpp                                     //
// Handles the entry point of the application,        //
// creates the window and initializes the GLContext   //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//

// Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "HelpLib.h"

// Header file for our OpenGL functions
#include "Renderer.h"

void main(int argc, char* argv[])
{
    // This is the entry point of the application.
    // This function initializes GLUT, creates the window
    // and sets the callbacks for several GLUT functions
    // which calls functions necessary for rendering a frame,
    // reshaping the window and handling key and mouse events

    int width = 500;
    int height = 500;

    int xPos = 50;
    int yPos = 50;

    std::string windowTitle = "OpenGL Multiple Lights Example";

    // glutInit initializes the GLUT library.
    // if something goes wrong, this will terminate with an error message
    glutInit(&argc, argv);

    // set OpenGL 3.3 Context as the default
    glutInitContextVersion(3, 3);

    // Do not include any deprecated features (meaningless for OpenGL3.2+ core)
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    // Do not include any features removed in previous versions (fixed-function pipeline, etc)
    glutInitContextProfile(GLUT_CORE_PROFILE);

    // inform FREEGLUT that we want to leave the main loop when the X button is pressed
    // this is CRITICAL since we need to release any allocated memory before
    // exiting the application
    glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
        );

    // glutInitDisplayMode tells GLUT what sort of rendering context we want.
    // Here we initialize the GL Context with double buffering enabled
    // and RGBA pixel type.
    // Multiple options are asked by OR-ing the parameters using the | operator
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

    // glutInitWindowSize tells GLUT the dimensions of the window we want to create
    glutInitWindowSize(width, height);

    // glutInitWindowPosition sets the parameters for the place on our screen we want
    // the window to appear on.
    // In this case the top left corner of the window will appear at position 50,50 of our screen
    glutInitWindowPosition(xPos, yPos);

    // glutCreateWindow will create a window based on the setting set above
    // with the name passed as an argument
    glutCreateWindow(windowTitle.c_str());

    // Callbacks for the GL and GLUT events:

    // Call back for the rendering function.
    // Each time glutPostRedisplay() is called, the function passed as an
    // argument here is called to redraw the window.
    glutDisplayFunc(Render);

    // The reshape callback function is called when the window changes size
    // (so we can adjust our viewports and redraw stuff)
    glutReshapeFunc(Resize);

    // Input Callbacks

    // Callback for keyboard control
    // Each time a keyboard key is pressed, GLUT calls the keyboard function
    glutKeyboardFunc(Keyboard);

    // This callback is responsible for handling special keys (e.g: arrow keys)
    glutSpecialFunc(KeyboardSpecial);

    // This callback is responsible for handling mouse events
    glutMouseFunc(Mouse);

    // This callback is responsible for handling mouse motion
    glutMotionFunc(MouseMotion);

    // This callback is responsible for handling idle events (when no events are being received)
    //glutIdleFunc(Idle);

    // This callback redraws the sync every 16ms (62.5fps) in order to fix issues where vsync is
    // disabled
    glutTimerFunc(0, TimerSync, 16);

    // Now initialize our renderer settings (if any)
    // InitializeRenderer() is a function of Renderer.cpp which takes care of some one-off settings
    // before we go into the main rendering loop
    bool ok = InitializeRenderer();

    // We are done
    // Now enter main event handling loop
    if (ok) glutMainLoop();

    // Release any allocated data
    ReleaseGLUT();

    PrintToOutputWindow("%s", "Goodbye!!");
    exit(EXIT_SUCCESS);
}