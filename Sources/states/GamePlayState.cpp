// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Common.h"
#include "Player.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    GamePlayState::GamePlayState (StateManager* stateManager)
        : State ("GAMEPLAY")
        , m_StateManager (stateManager)
        , m_Player (nullptr)
    {
    }

    //--------------------------------------------------------------------------------------------------

    GamePlayState::~GamePlayState ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Initialize ()
    {
        m_Player = new Player (m_StateManager->GetScreen ());
        m_Player->Initialize ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy ()
    {
        SAFE_DELETE (m_Player);

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        m_StateManager->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 60));
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_Player->ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (double deltaTime)
    {
        m_Player->Update (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime)
    {
        m_Player->Render (deltaTime);

        m_StateManager->GetScreen ()->GetFont ().DrawText (FONT_NAME_MAIN, al_map_rgb (255, 255, 255), 200, 200, "Robot Tale");
    }

    //--------------------------------------------------------------------------------------------------
}
