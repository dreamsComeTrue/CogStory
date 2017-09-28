// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "EditorFrame.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    EditorFrame::EditorFrame (Point size)
      : wxFrame (NULL, wxID_ANY, "Editor", wxPoint (0, 0), wxSize (size.X, size.Y))
    {
    }

    //--------------------------------------------------------------------------------------------------
}
