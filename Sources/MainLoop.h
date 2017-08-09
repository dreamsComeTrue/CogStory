// Copyright 2017 Dominik 'dreamsComeTrue' Jasiński. All Rights Reserved.

#ifndef __MAIN_LOOP_H__
#define __MAIN_LOOP_H__

#include "Lifecycle.h"
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
        StateManager* GetStateManager ();
        SceneManager* GetSceneManager ();
        ScriptManager* GetScriptManager ();
        TweenManager* GetTweenManager ();

    private:
        void InitializeStates ();
        void DestroyStates ();

    private:
        Screen* m_Screen;
        StateManager m_StateManager;
        SceneManager m_SceneManager;
        ScriptManager m_ScriptManager;
        TweenManager m_TweenManager;

        MainMenuState* m_MainMenuState;
        GamePlayState* m_GamePlayState;
        EditorState* m_EditorState;

        bool m_IsRunning;
    };
}

#endif //   __MAIN_LOOP_H__
