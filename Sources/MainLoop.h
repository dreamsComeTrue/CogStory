// Copyright 2017-2019 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_LOOP_H__
#define __MAIN_LOOP_H__

#include "AtlasManager.h"
#include "AudioManager.h"
#include "Lifecycle.h"
#include "PhysicsManager.h"
#include "SceneManager.h"
#include "ScriptManager.h"
#include "StateManager.h"
#include "TweenManager.h"

namespace aga
{
    class Screen;
    class MainMenuState;
    class GamePlayState;
    class EditorState;

    class MainLoop : public Lifecycle
    {
    public:
        MainLoop (Screen* screen);
        virtual ~MainLoop ();
        bool Initialize ();
        bool Destroy ();

        void Start ();
        void Exit ();

        Screen* GetScreen ();
        StateManager& GetStateManager ();
        SceneManager& GetSceneManager ();
        ScriptManager& GetScriptManager ();
        PhysicsManager& GetPhysicsManager ();
        TweenManager& GetTweenManager ();
        AudioManager& GetAudioManager ();
        AtlasManager& GetAtlasManager ();
        
        void SetSpeedMultiplier (float speed);
        float GetSpeedMultiplier ();

    private:
        void InitializeStates ();
        void DestroyStates ();
        
        void LoadConfig ();
        void SaveConfig ();

    private:
        Screen* m_Screen;
        AudioManager m_AudioManager;
        StateManager m_StateManager;
        SceneManager m_SceneManager;
        ScriptManager m_ScriptManager;
        PhysicsManager m_PhysicsManager;
        TweenManager m_TweenManager;
        AtlasManager m_AtlasManager;

        MainMenuState* m_MainMenuState;
        GamePlayState* m_GamePlayState;
        EditorState* m_EditorState;

        bool m_IsRunning;
        float m_SpeedMultiplier;
    };
}

#endif //   __MAIN_LOOP_H__
