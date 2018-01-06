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
      , m_Direction (0)
      , m_StepsCounter (0.0f)
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

    float randMove;

    bool NPCActor::Update (float deltaTime)
    {
        if (m_StepsCounter <= 0.0f)
        {
            m_StepsCounter = RandInRange (0, 3);
            m_Direction = RandInRange (0, 4);
            randMove = RandInRange (0, 0.2f);
        }

        m_StepsCounter -= deltaTime;

        switch (m_Direction)
        {
            case 1:
                Move (0.0f, -randMove);
                break;

            case 2:
                Move (randMove, 0.0f);
                break;

            case 3:
                Move (0.0f, randMove);
                break;

            case 4:
                Move (-randMove, 0.0f);
                break;
        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------
}
