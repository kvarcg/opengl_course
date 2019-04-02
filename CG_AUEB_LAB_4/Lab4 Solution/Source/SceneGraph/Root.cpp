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

// includes ////////////////////////////////////////
#include "../HelpLib.h" // - Library for including GL libraries, checking for OpenGL errors, writing to Output window, etc.
#include "Root.h"       // - Header file for the Root class

// defines /////////////////////////////////////////


void Root::Draw(void)
{
    GroupNode::Draw();
}

// Constructor
Root::Root():
GroupNode("root")
{
    m_parent = nullptr;
    m_root = this;
}

// Destructor
Root::~Root()
{

}

// other functions
void Root::Update()
{
    GroupNode::Update();
}

void Root::Init()
{
    GroupNode::Init();

    SetRoot(this);
}

void Root::SetRoot(GroupNode* gnd)
{
    for (unsigned int i=0; i<gnd->children.size();i++)
    {
        Node* childnode = gnd->children.at(i);
        childnode->SetWorld(this);

        GroupNode* group = dynamic_cast<GroupNode*>(childnode);
        if (group)
        {
            if (group->children.size() > 0)
            {
                SetRoot(group);
            }
        }
    }
}

// eof ///////////////////////////////// class Root