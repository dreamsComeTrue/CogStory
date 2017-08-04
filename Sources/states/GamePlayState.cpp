// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Common.h"
#include "MainLoop.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "ScriptManager.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    GamePlayState::GamePlayState (MainLoop* mainLoop)
      : State (mainLoop, "GAMEPLAY")
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
        Lifecycle::Initialize ();

        Scene* scene00 = Scene::LoadScene (m_MainLoop->GetSceneManager (), GetResourcePath (SCENE_0_0));

        m_MainLoop->GetSceneManager ()->AddScene (SCENE_0_0, scene00);

        Point pos = scene00->GetSpawnPoint ("DEFAULT");
        m_MainLoop->GetSceneManager ()->GetPlayer ().SetPosition (pos);

        m_MainLoop->GetSceneManager ()->SetActiveScene (scene00);

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (60, 60, 70));
        m_MainLoop->GetScreen ()->SetBackgroundColor (al_map_rgb (50, 60, 100));
        m_MainLoop->GetSceneManager ()->SetActiveScene (m_MainLoop->GetSceneManager ()->GetActiveScene ());

        Script* script = m_MainLoop->GetScriptManager ()->LoadScriptFromFile (GetResourcePath (SCRIPT_0_0), "home");
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave () {}

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, double deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (double deltaTime) {}

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (double deltaTime) {}

    //--------------------------------------------------------------------------------------------------
}
