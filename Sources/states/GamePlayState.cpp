// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    std::string GAMEPLAY_STATE_NAME = "GAMEPLAY_STATE";

    GamePlayState::GamePlayState (MainLoop* mainLoop)
        : State (mainLoop, GAMEPLAY_STATE_NAME)
    {
    }

    //--------------------------------------------------------------------------------------------------

    GamePlayState::~GamePlayState ()
    {
        if (!IsDestroyed ())
        {
            Destroy ();
        }
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Initialize ()
    {
        Lifecycle::Initialize ();

        m_CurrentScene = Scene::LoadScene (&m_MainLoop->GetSceneManager (), GetResourcePath (SCENE_0_0));

        m_MainLoop->GetSceneManager ().AddScene (SCENE_0_0, m_CurrentScene);
        m_MainLoop->GetSceneManager ().SetActiveScene (m_CurrentScene);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (50, 60, 100));
        m_MainLoop->GetSceneManager ().GetActiveScene ()->BeforeEnter ();

        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.4, 1.4, winSize.Width * 0.5, winSize.Height * 0.5);

#ifndef EDITOR_ENABLED
        m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (false);
#endif
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave () { m_MainLoop->GetSceneManager ().GetActiveScene ()->AfterLeave (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_MainLoop->GetSceneManager ().GetPlayer ().ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (double deltaTime)
    {
        m_MainLoop->GetSceneManager ().GetPlayer ().HandleInput (deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime) { m_MainLoop->GetSceneManager ().Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
