// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.
#include "NPCActor.h"
#include "Player.h"
#include "SceneManager.h"

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
        SetCheckOverlap (true);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    float randMove;

    bool NPCActor::Update (float deltaTime)
    {
        Actor::Update (deltaTime);

        if (m_StepsCounter <= 0.0f)
        {
            m_StepsCounter = RandInRange (0, 3);
            m_Direction = RandInRange (0, 4);
            randMove = RandInRange (0, 0.2f);
        }

        m_StepsCounter -= deltaTime;

        //        switch (m_Direction)
        //        {
        //            case 1:
        //                Move (0.0f, -randMove);
        //                break;

        //            case 2:
        //                Move (randMove, 0.0f);
        //                break;

        //            case 3:
        //                Move (0.0f, randMove);
        //                break;

        //            case 4:
        //                Move (-randMove, 0.0f);
        //                break;
        //        }
    }

    //--------------------------------------------------------------------------------------------------

    std::string NPCActor::GetTypeName () { return TypeName; }

    //--------------------------------------------------------------------------------------------------

    void NPCActor::BeginOverlap (Entity* entity)
    {
        if (entity->GetTypeName () == Player::TypeName)
        {
            m_SceneManager->GetSpeechFrameManager ().AddSpeechFrame (
              "d",
              "Hello [color=GREEN]PTR4617[/color]!!\nDo you have [color=LIGHTBLUE]blue[/color] key to open this door?",
              Point (400, 20),
              20,
              3,
              true);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void NPCActor::EndOverlap (Entity* entity) {}

    //--------------------------------------------------------------------------------------------------
}
