// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "Common.h"
#include "MainLoop.h"
#include "MainMenuState.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

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

        m_MainLoop->GetSceneManager ().SetActiveScene ("0_home/0_0_home.scn", false);

        m_MainLoop->GetSceneManager ().GetPlayer ().TemplateBounds.Pos
            = m_MainLoop->GetSceneManager ().GetPlayer ().GetPosition ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        if (m_MainLoop->GetStateManager ().GetPreviousState () != nullptr
            && m_MainLoop->GetStateManager ().GetPreviousState ()->GetName () == MAIN_MENU_STATE_NAME)
        {
            return;
        }

        m_MainLoop->GetSceneManager ().GetSpeechFrameManager ().Clear ();
        m_MainLoop->GetSceneManager ().GetCamera ().ClearTransformations ();

        //  Initial scale in gameplay twice big as normal
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        float scale = 2.0f;
        m_MainLoop->GetSceneManager ().GetCamera ().Scale (scale, scale, winSize.Width * 0.5, winSize.Height * 0.5);

        m_MainLoop->GetSceneManager ().GetActiveScene ()->BeforeEnter ();

        //  Reset camera to player
        m_MainLoop->GetSceneManager ().GetPlayer ().Move (0, 0.00001);

#ifndef EDITOR_ENABLED
        m_MainLoop->GetSceneManager ().GetActiveScene ()->SetDrawPhysData (false);
#else
        Point playerPos = m_MainLoop->GetSceneManager ().GetPlayer ().TemplateBounds.Pos;
        m_MainLoop->GetSceneManager ().GetPlayer ().SetPosition (playerPos);
#endif
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave () { m_MainLoop->GetSceneManager ().GetActiveScene ()->AfterLeave (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        m_MainLoop->GetSceneManager ().GetPlayer ().ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (float deltaTime)
    {
        SceneManager& sceneManager = m_MainLoop->GetSceneManager ();

        if (!sceneManager.IsTransitioning () && !sceneManager.GetPlayer ().IsPreventInput ())
        {
            sceneManager.GetPlayer ().HandleInput (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (float deltaTime) { m_MainLoop->GetSceneManager ().Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
