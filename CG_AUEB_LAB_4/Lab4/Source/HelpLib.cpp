//----------------------------------------------------//
//                                                    //
// File: HelpLib.cpp                                  //
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

#include "HelpLib.h" // - Header file for the HelpLib
#include <iostream>

bool PrintToOutputWindow(const char * fmt, ...)
{
    char* text;                                    // Holds Our String
    int len;
    va_list args;                                // Pointer To List Of Arguments

    if (fmt == NULL)                            // If There's No Text
        return false;                            // Do Nothing

    va_start(args, fmt);                        // Parses The String For Variables
    len = _vscprintf(fmt, args) + 2;            // _vscprintf doesn't count
    text = (char*)malloc(len * sizeof(char));
    vsprintf_s(text, len, fmt, args);            // And Converts Symbols To Actual Numbers
    va_end(args);                                // Results Are Stored In Text

    std::cout << text << std::endl;

    text[len - 2] = NEW_LINE_CHAR;
    text[len - 1] = NULL_TERMINATED_CHAR;
    OutputDebugStringA(LPCSTR(text));

    SAFE_DELETE(text)

    return true;
}

bool glError(void)
{
    //return false;
    unsigned int _enum = glGetError();
    if (_enum != GL_NO_ERROR)
    {
        std::string er = "";
        switch (_enum)
        {
        case 0x500:        er = "GL_INVALID_ENUM 0x0500\nAn unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag."; break;
        case 0x501:        er = "GL_INVALID_VALUE 0x0501\nA numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag."; break;
        case 0x502:        er = "GL_INVALID_OPERATION 0x0502\nThe specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag."; break;
        case 0x503:        er = "GL_STACK_OVERFLOW 0x0503\nThis command would cause a stack overflow. The offending command is ignored and has no other side effect than to set the error flag."; break;
        case 0x504:        er = "GL_STACK_UNDERFLOW 0x0504\nThis command would cause a stack underflow. The offending command is ignored and has no other side effect than to set the error flag."; break;
        case 0x505:        er = "GL_OUT_OF_MEMORY 0x0505\nThere is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded."; break;
        case 0x8031:    er = "GL_TABLE_TOO_LARGE 0x8031\nThe specified table exceeds the implementation's maximum supported table size.  The offending command is ignored and has no other side effect than to set the error flag."; break;
        default:        er = "Undefined GL error"; break;
        };
        PrintToOutputWindow("%s", er.c_str());
        return true;
    }
    return false;
}

bool readFile(std::string& filename, char** data)
{
    FILE* File = NULL;
    fopen_s(&File, filename.c_str(), "r");
    if (File != NULL)
    {
        size_t count = 0;
        fseek(File, 0, SEEK_END);
        count = ftell(File);
        rewind(File);
        if (count > 0)
        {
            *data = new char[count + 1];
            memset(*data, 0, count);
            count = fread(*data, sizeof(char), count, File);
            (*data)[count] = NULL_TERMINATED_CHAR;
            fclose(File);
            return true;
        }
        fclose(File);
        return false;
    }
    return false;
}