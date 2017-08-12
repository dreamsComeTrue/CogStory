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
      : m_SceneManager (this)
      , m_StateManager (this)
      , m_ScriptManager (this)
      , m_TweenManager (this)
      , m_EditorState (nullptr)
      , m_GamePlayState (nullptr)
      , m_MainMenuState (nullptr)
      , m_Screen (screen)
      , m_IsRunning (true)
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
        m_SceneManager.Initialize ();
        m_ScriptManager.Initialize ();
        m_StateManager.Initialize ();
        m_TweenManager.Initialize ();

        InitializeStates ();

        m_StateManager.SetActiveState (m_EditorState);

        m_Screen->ProcessEventFunction = [&](ALLEGRO_EVENT* event) { m_StateManager.ProcessEvent (event, m_Screen->GetDeltaTime ()); };

        m_Screen->RenderFunction = [&]() {
            m_SceneManager.Render (m_Screen->GetDeltaTime ());
            m_StateManager.Render (m_Screen->GetDeltaTime ());
        };

        Lifecycle::Initialize ();
    }

    //--------------------------------------------------------------------------------------------------

    bool MainLoop::Destroy ()
    {
        DestroyStates ();

        m_TweenManager.Destroy ();
        m_SceneManager.Destroy ();
        m_ScriptManager.Destroy ();
        m_StateManager.Destroy ();

        Lifecycle::Destroy ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::InitializeStates ()
    {
        m_MainMenuState = new MainMenuState (this);
        m_MainMenuState->Initialize ();
        m_StateManager.RegisterState (m_MainMenuState);

        m_GamePlayState = new GamePlayState (this);
        m_GamePlayState->Initialize ();
        m_StateManager.RegisterState (m_GamePlayState);

        m_EditorState = new EditorState (this);
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

        while (m_IsRunning)
        {
            double newTime = al_get_time ();
            double deltaTime = (newTime - oldTime);
            oldTime = newTime;

            m_TweenManager.Update (deltaTime);
            m_SceneManager.Update (deltaTime);

            if (!m_StateManager.Update (deltaTime))
            {
                break;
            }
        }
    }

    //--------------------------------------------------------------------------------------------------

    void MainLoop::Exit () { m_IsRunning = false; }

    //--------------------------------------------------------------------------------------------------

    Screen* MainLoop::GetScreen () { return m_Screen; }

    //--------------------------------------------------------------------------------------------------

    StateManager* MainLoop::GetStateManager () { return &m_StateManager; }

    //--------------------------------------------------------------------------------------------------

    SceneManager* MainLoop::GetSceneManager () { return &m_SceneManager; }

    //--------------------------------------------------------------------------------------------------

    ScriptManager* MainLoop::GetScriptManager () { return &m_ScriptManager; }

    //--------------------------------------------------------------------------------------------------

    TweenManager* MainLoop::GetTweenManager () { return &m_TweenManager; }

    //--------------------------------------------------------------------------------------------------
}
