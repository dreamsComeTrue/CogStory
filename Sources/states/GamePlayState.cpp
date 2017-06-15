// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Common.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    GamePlayState::GamePlayState (StateManager* stateManager)
        : State ("GAMEPLAY")
        , m_StateManager (stateManager)
        , m_SceneManager (stateManager->GetScreen ())
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
        m_SceneManager.Initialize ();

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy ()
    {
        m_SceneManager.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        m_StateManager->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave ()
    {
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (double deltaTime)
    {
        m_SceneManager.Update (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime)
    {
        m_SceneManager.Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------
}
