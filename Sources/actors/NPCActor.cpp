// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "NPCActor.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::TypeName = "NPC";

    //--------------------------------------------------------------------------------------------------
    //--------------------------------------------------------------------------------------------------

    NPCActor::NPCActor (SceneManager* sceneManager)
        : Actor (sceneManager)
    {
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Initialize ()
    {
        Actor::Initialize ();
        Animable::Initialize ("menu_ui", "cog");

        Bounds.SetSize ({ 64, 64 });
        SetCheckOverlap (true);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        return true;
    }

    //--------------------------------------------------------------------------------------------------
}
