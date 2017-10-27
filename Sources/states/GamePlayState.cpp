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

        Lifecycle::Destroy ();
    }

    //--------------------------------------------------------------------------------------------------

    Scene* scene00;
    bool GamePlayState::Initialize ()
    {
        Lifecycle::Initialize ();

        scene00 = Scene::LoadScene (&m_MainLoop->GetSceneManager (), GetResourcePath (SCENE_0_0));

        m_MainLoop->GetSceneManager ().AddScene (SCENE_0_0, scene00);
        m_MainLoop->GetSceneManager ().SetActiveScene (scene00);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        Point pos = scene00->GetSpawnPoint ("DEFAULT");
        m_MainLoop->GetSceneManager ().GetPlayer ().SetPosition (pos);

        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (50, 60, 100));
        m_MainLoop->GetSceneManager ().SetActiveScene (m_MainLoop->GetSceneManager ().GetActiveScene ());
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        m_MainLoop->GetSceneManager ().GetCamera ().Scale (1.4, 1.4, winSize.Width * 0.5, winSize.Height * 0.5);

        m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (false);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime)
    {
        m_MainLoop->GetSceneManager ().GetPlayer ().ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (double deltaTime) { m_MainLoop->GetSceneManager ().GetPlayer ().HandleInput (deltaTime); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime) { m_MainLoop->GetSceneManager ().Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
