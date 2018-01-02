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

        m_Image = al_load_bitmap (GetResourcePath (ResourceID::GFX_MENU_COG).c_str ());
        Bounds.SetSize ({ 64, 64 });

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool NPCActor::Update (float deltaTime)
    {
        if (RandZeroToOne () < 0.5f)
        {
            float randX = RandInRange (-1, 1);
            float randY = RandInRange (-1, 1);

            Move (randX, randY);
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
