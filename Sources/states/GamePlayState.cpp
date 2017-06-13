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

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event)
    {
        if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
        {
        }
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update ()
    {
        m_Player->Update ();
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render ()
    {
        m_Player->Render ();
    }

    //--------------------------------------------------------------------------------------------------
}
