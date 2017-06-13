// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainLoop.h"
#include "Common.h"
#include "Screen.h"
#include "StateManager.h"

#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    MainLoop::MainLoop (Screen* screen)
        : m_Screen (screen)
    {
    }

    //--------------------------------------------------------------------------------------------------

    MainLoop::~MainLoop ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MainLoop::Initialize ()
    {
        m_StateManager = new StateManager (m_Screen);
        m_StateManager->Initialize ();

        InitializeStates ();

        m_StateManager->SetActiveState (m_GamePlayState);

        m_Screen->ProcessEventFunction = [&](ALLEGRO_EVENT* event) {
            m_StateManager->ProcessEvent (event);
        };

        m_Screen->RenderFunction = [&]() {
            m_StateManager->Render ();
        };

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MainLoop::Destroy ()
    {
        DestroyStates ();

        SAFE_DELETE (m_StateManager);

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::InitializeStates ()
    {
        m_MainMenuState = new MainMenuState (m_StateManager);
        m_MainMenuState->Initialize ();
        m_StateManager->RegisterState (m_MainMenuState);

        m_GamePlayState = new GamePlayState (m_StateManager);
        m_GamePlayState->Initialize ();
        m_StateManager->RegisterState (m_GamePlayState);
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::DestroyStates ()
    {
        SAFE_DELETE (m_MainMenuState);
        SAFE_DELETE (m_GamePlayState);
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::Start ()
    {
        while (true)
        {
            if (!m_StateManager->Update ())
            {
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
