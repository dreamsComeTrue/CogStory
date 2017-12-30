// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "NPCActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    NPCActor::NPCActor (SceneManager* sceneManager)
      : Actor (sceneManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::GetTypeName () { return "NPC"; }

    //--------------------------------------------------------------------------------------------------
}
