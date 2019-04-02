//----------------------------------------------------//
//                                                    //
// File: Root.h                                       //
// This scene graph is a basic example for the        //
// object relational management of the scene          //
// This holds the root node                           //
//                                                    //
// Author:                                            //
// Kostas Vardis                                      //
//                                                    //
// These files are provided as part of the BSc course //
// of Computer Graphics at the Athens University of   //
// Economics and Business (AUEB)                      //
//                                                    //
//----------------------------------------------------//
#ifndef ROOT_H
#define ROOT_H

#pragma once
//using namespace

// includes ////////////////////////////////////////
#include "GroupNode.h"

// defines /////////////////////////////////////////


// forward declarations ////////////////////////////


// class declarations //////////////////////////////

class Root : public GroupNode
{
protected:
    // protected variable declarations
    glm::mat4x4                         m_view_mat;
    glm::mat4x4                         m_projection_mat;

    // protected function declarations

private:
    // private variable declarations
    void                                SetRoot(GroupNode* gnd);

    // private function declarations


public:
    // Constructor
    Root(void);

    // Destructor
    ~Root(void);

    // public function declarations
    void                                Init(void);
    void                                Update(void);
    void                                Draw(void);

    // get functions
    glm::mat4x4&                        GetViewMat(void)                                {return m_view_mat;}
    glm::mat4x4&                        GetProjectionMat(void)                          {return m_projection_mat;}

    // set functions
    void                                SetViewMat(glm::mat4x4& mat)                    {m_view_mat = mat;}
    void                                SetProjectionMat(glm::mat4x4& mat)              {m_projection_mat = mat;}
};

#endif //ROOT_H

// eof ///////////////////////////////// class Root