// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "MainLoop.h"
#include "Common.h"
#include "Screen.h"

#include "states/EditorState.h"
#include "states/GamePlayState.h"
#include "states/MainMenuState.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    MainLoop::MainLoop (Screen* screen)
        : m_StateManager (m_Screen)
        , m_EditorState (nullptr)
        , m_GamePlayState (nullptr)
        , m_MainMenuState (nullptr)
        , m_Screen (screen)
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
        m_StateManager.Initialize ();

        InitializeStates ();

        m_StateManager.SetActiveState (m_EditorState);

        m_Screen->ProcessEventFunction = [&](ALLEGRO_EVENT* event) 
        {
            m_StateManager.ProcessEvent (event, m_Screen->GetDeltaTime ());
        };

        m_Screen->RenderFunction = [&]() 
        {
            m_StateManager.Render (m_Screen->GetDeltaTime ());
        };

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MainLoop::Destroy ()
    {
        DestroyStates ();

        m_StateManager.Destroy ();

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::InitializeStates ()
    {
        m_MainMenuState = new MainMenuState (&m_StateManager);
        m_MainMenuState->Initialize ();
        m_StateManager.RegisterState (m_MainMenuState);

        m_GamePlayState = new GamePlayState (&m_StateManager);
        m_GamePlayState->Initialize ();
        m_StateManager.RegisterState (m_GamePlayState);

        m_EditorState = new EditorState (&m_StateManager);
        m_EditorState->Initialize ();
        m_StateManager.RegisterState (m_EditorState);
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::DestroyStates ()
    {
        SAFE_DELETE (m_MainMenuState);
        SAFE_DELETE (m_EditorState);
        SAFE_DELETE (m_GamePlayState);
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::Start ()
    {
        double oldTime = al_get_time ();

        while (true)
        {
            double newTime = al_get_time ();
            double deltaTime = (newTime - oldTime);
            oldTime = newTime;

            if (!m_StateManager.Update (deltaTime))
            {
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------
}
