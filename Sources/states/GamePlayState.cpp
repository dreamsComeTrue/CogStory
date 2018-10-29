// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#include "GamePlayState.h"
#include "AudioStream.h"
#include "MainLoop.h"
#include "MainMenuState.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Screen.h"
#include "Script.h"
#include "StateManager.h"

namespace aga
{
    //--------------------------------------------------------------------------------------------------

    GamePlayState::GamePlayState (MainLoop* mainLoop)
        : State (mainLoop, GAMEPLAY_STATE_NAME)
        , m_AudioWasEnabled (true)
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

        Script* masterScript
            = m_MainLoop->GetScriptManager ().LoadScriptFromFile (GetDataPath () + "/scripts/Master.script", "master");
        masterScript->Run ("void Start ()");

        m_MainLoop->GetSceneManager ().GetPlayer ()->TemplateBounds.Pos
            = m_MainLoop->GetSceneManager ().GetPlayer ()->GetPosition ();

        return true;
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::Destroy () { return Lifecycle::Destroy (); }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::BeforeEnter ()
    {
        m_MainLoop->GetAudioManager ().SetEnabled (m_AudioWasEnabled);

        al_hide_mouse_cursor (m_MainLoop->GetScreen ()->GetDisplay ());

        if (m_MainLoop->GetStateManager ().GetPreviousState () != nullptr
            && m_MainLoop->GetStateManager ().GetPreviousState ()->GetName () == MAIN_MENU_STATE_NAME)
        {
            return;
        }

        SceneManager& sceneManager = m_MainLoop->GetSceneManager ();

        sceneManager.GetSpeechFrameManager ().Clear ();
        sceneManager.GetCamera ().ClearTransformations ();

        //  Initial scale in gameplay twice big as normal
        const Point winSize = m_MainLoop->GetScreen ()->GetWindowSize ();
        const float SCREEN_SCALE = 2.0f;
        sceneManager.GetCamera ().Scale (SCREEN_SCALE, SCREEN_SCALE, winSize.Width * 0.5, winSize.Height * 0.5);

        //  Re-init current initial scene everytime we start GamePlayState
        sceneManager.SetActiveScene (sceneManager.GetActiveScene ());
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::AfterLeave ()
    {
        m_MainLoop->GetSceneManager ().GetActiveScene ()->AfterLeave ();
        m_MainLoop->GetAudioManager ().SetEnabled (true);
    }

    //--------------------------------------------------------------------------------------------------

    bool GamePlayState::ProcessEvent (ALLEGRO_EVENT* event, float deltaTime)
    {
        if (event->type == ALLEGRO_EVENT_KEY_UP)
        {
            if (event->keyboard.keycode == ALLEGRO_KEY_M)
            {
                m_MainLoop->GetAudioManager ().SetEnabled (!m_MainLoop->GetAudioManager ().IsEnabled ());
                m_AudioWasEnabled = m_MainLoop->GetAudioManager ().IsEnabled ();
            }

            if (m_MainLoop->GetAudioManager ().IsEnabled ())
            {
                AudioStream* audioStream = m_MainLoop->GetSceneManager ().GetActiveScene ()->GetSceneAudioStream ();

                if (audioStream)
                {
                    audioStream->Play ();
                }
            }
        }

        return m_MainLoop->GetSceneManager ().GetPlayer ()->ProcessEvent (event, deltaTime);
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Update (float deltaTime)
    {
        SceneManager& sceneManager = m_MainLoop->GetSceneManager ();

        if (!sceneManager.IsTransitioning () && !sceneManager.GetPlayer ()->IsPreventInput ())
        {
            sceneManager.GetPlayer ()->HandleInput (deltaTime);
        }
    }

    //--------------------------------------------------------------------------------------------------

    void GamePlayState::Render (float deltaTime) { m_MainLoop->GetSceneManager ().Render (deltaTime); }

    //--------------------------------------------------------------------------------------------------
}
