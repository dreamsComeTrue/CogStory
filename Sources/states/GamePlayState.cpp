// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Camera.h"
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
        , m_SceneManager (stateManager->GetScreen ())
        , m_Player (nullptr)
        , m_Camera (nullptr)
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

        m_Player = new Player (m_StateManager->GetScreen ());
        m_Player->Initialize ();

        m_Camera = new Camera (m_StateManager->GetScreen ());
        const Point& screenSize = m_StateManager->GetScreen ()->GetScreenSize ();
        Point& playerSize = m_Player->GetSize ();
        m_Camera->SetOffset (screenSize.Width * 0.5 - playerSize.Width * 0.5,
            screenSize.Height * 0.5 - playerSize.Height * 0.5);

        m_Player->MoveCallback = [&](double dx, double dy) {
            m_Camera->Move (-dx, -dy);
        };

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy ()
    {
        SAFE_DELETE (m_Camera);
        SAFE_DELETE (m_Player);

        m_SceneManager.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        int pigment = 40;
        m_StateManager->GetScreen ()->SetBackgroundColor (al_map_rgb (pigment, pigment, pigment));
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
        m_Camera->Update (deltaTime);

        m_SceneManager.Update (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime)
    {
        m_Player->Render (deltaTime);
        m_SceneManager.Render (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------
}
